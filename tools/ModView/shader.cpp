// Filename:-	shader.cpp
//
// guess...


#include "stdafx.h"

#include "includes.h"
#include "R_Common.h"
#include "R_Image.h"
#include "files.h"
#include "shader.h"
#include "StringUtils.h"

shaderCommands_t tess;

static char *s_shaderText = NULL;

static	char	com_token[MAX_TOKEN_CHARS];
static	char	com_parsename[MAX_TOKEN_CHARS];
static	int		com_lines;

static void COM_BeginParseSession( const char *name )
{
	com_lines = 0;
	Com_sprintf(com_parsename, sizeof(com_parsename), "%s", name);
}

/*
==============
COM_Parse

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/
static char *SkipWhitespace( char *data, qboolean *hasNewLines ) {
	int c;

	while( (c = *data) <= ' ') {
		if( !c ) {
			return NULL;
		}
		if( c == '\n' ) {
			com_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

static char *COM_ParseExt( char **data_p, qboolean allowLineBreaks )
{
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	char *data;
    
	data = *data_p;
	len = 0;
	com_token[0] = 0;
    
	// make sure incoming data is valid
	if ( !data )
	{
		*data_p = NULL;
		return com_token;
	}
    
	while ( 1 )
	{
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data )
		{
			*data_p = NULL;
			return com_token;
		}
		if ( hasNewLines && !allowLineBreaks )
		{
			*data_p = data;
			return com_token;
		}
        
		c = *data;
        
		// skip double slash comments
		if ( c == '/' && data[1] == '/' )
		{
			data += 2;
			while (*data && *data != '\n') {
				data++;
			}
		}
		// skip /* */ comments
		else if ( c=='/' && data[1] == '*' )
		{
			data += 2;
			while ( *data && ( *data != '*' || data[1] != '/' ) )
			{
				data++;
			}
			if ( *data )
			{
				data += 2;
			}
		}
		else
		{
			break;
		}
	}
    
	// handle quoted strings
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				*data_p = ( char * ) data;
				return com_token;
			}
			if (len < MAX_TOKEN_CHARS)
			{
				com_token[len] = c;
				len++;
			}
		}
	}
    
	// parse a regular word
	do
	{
		if (len < MAX_TOKEN_CHARS)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
		if ( c == '\n' )
			com_lines++;
	} while (c>32);
    
	if (len == MAX_TOKEN_CHARS)
	{
        //		Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	}
	com_token[len] = 0;
    
	*data_p = ( char * ) data;
	return com_token;
}

/*
=================
SkipBracedSection

The next token should be an open brace.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
static void SkipBracedSection (char **program) {
	char			*token;
	int				depth;

	depth = 0;
    
    COM_BeginParseSession ("SkipBracedSection");
	do {
		token = COM_ParseExt( program, qtrue );
		if( token[1] == 0 ) {
			if( token[0] == '{' ) {
				depth++;
			}
			else if( token[0] == '}' ) {
				depth--;
			}
		}
	} while( depth && *program );
}


/*
====================
FindShaderInShaderText

Scans the combined text description of all the shader files for
the given shader name.

return NULL if not found

If found, it will return a valid shader
=====================
*/
static char *FindShaderInShaderText( const char *shadername ) {
#ifdef SHADERTEXTHASH

	char *token, *p;
	int i, hash;

	hash = generateHashValue(shadername, MAX_SHADERTEXT_HASH);

    COM_BeginParseSession (shadername);
	for (i = 0; shaderTextHashTable[hash][i]; i++) {
		p = shaderTextHashTable[hash][i];
        
		token = COM_ParseExt(&p, qtrue);
		if ( !Q_stricmp( token, shadername ) ) {
			return p;
		}
	}
	return NULL;
#else
	char *p = s_shaderText;
	char *token;

	if ( !p ) {
		return NULL;
	}

	// look for label
    COM_BeginParseSession(shadername);
	while ( 1 ) {
		token = COM_ParseExt( &p, qtrue );
		if ( token[0] == 0 ) {
			break;
		}

		if ( !Q_stricmp( token, shadername ) ) {
			return p;
		}
		else {
			// skip the definition
			SkipBracedSection( &p );
		}
	}

	return NULL;
#endif
}




/*
====================
ScanAndLoadShaderFiles

Finds and loads all .shader files, combining them into
a single large text block that can be scanned for shader names
=====================
*/
#define	MAX_SHADER_FILES	1024

typedef map<string,string>	ShadersFoundAndFilesPicked_t;
ShadersFoundAndFilesPicked_t ShadersFoundAndFilesPicked;

void KillAllShaderFiles(void)
{
	SAFEFREE(s_shaderText);
	ShadersFoundAndFilesPicked.clear();
}


void ScanAndLoadShaderFiles( void )
{
	if (s_shaderText == NULL && strlen(gamedir) > 0)
	{
		char **shaderFiles;
		char *buffers[MAX_SHADER_FILES];
	#ifdef SHADERTEXTHASH
		char *p, *oldp, *token, *hashMem;
		int shaderTextHashTableSizes[MAX_SHADERTEXT_HASH], hash, size;
	#endif
		int numShaders;
		int i;
		long sum = 0;

		#define sSHADER_DIR va("%sshaders",gamedir)

		// scan for shader files
		shaderFiles =	//ri.FS_ListFiles( "shaders", ".shader", &numShaders );
						Sys_ListFiles(	sSHADER_DIR,	// const char *directory, 
										".shader",	// const char *extension, 
										NULL,		// char *filter, 
										&numShaders,// int *numfiles, 
										qfalse		// qboolean wantsubs 
										);

		if ( !shaderFiles || !numShaders )
		{
			if (!bXMenPathHack)
			{
				ri.Printf( PRINT_WARNING, "WARNING: no shader files found in '%s'\n",sSHADER_DIR );
			}
			s_shaderText = CopyString("// blank shader file to avoid re-scanning shaders\n");
			return;
		}

		if ( numShaders > MAX_SHADER_FILES ) {
			numShaders = MAX_SHADER_FILES;
		}

		// load and parse shader files
		for ( i = 0; i < numShaders; i++ )
		{
			char filename[MAX_QPATH];

			Com_sprintf( filename, sizeof( filename ), "shaders/%s", shaderFiles[i] );
			StatusMessage( va("Loading shader %d/%d: \"%s\"...",i+1,numShaders,filename));

			//ri.Printf( PRINT_ALL, "...loading '%s'\n", filename );
			sum += ri.FS_ReadFile( filename, (void **)&buffers[i] );
			if ( !buffers[i] ) {
				ri.Error( ERR_DROP, "Couldn't load %s", filename );
			}
		}
		StatusMessage(NULL);

		// build single large buffer
		s_shaderText = (char *)ri.Hunk_Alloc( sum + numShaders*2 );

		// free in reverse order, so the temp files are all dumped
		for ( i = numShaders - 1; i >= 0 ; i-- ) {
			strcat( s_shaderText, "\n" );
			strcat( s_shaderText, buffers[i] );
			ri.FS_FreeFile( buffers[i] );
		}

		// free up memory
		//ri.FS_FreeFileList( shaderFiles );
		Sys_FreeFileList( shaderFiles );

	#ifdef SHADERTEXTHASH
		memset(shaderTextHashTableSizes, 0, sizeof(shaderTextHashTableSizes));
		size = 0;
		p = s_shaderText;
		// look for label
        COM_BeginParseSession ("ScanAndLoadShaderFiles");
		while ( 1 ) {
			token = COM_ParseExt( &p, qtrue );
			if ( token[0] == 0 ) {
				break;
			}

			hash = generateHashValue(token, MAX_SHADERTEXT_HASH);
			shaderTextHashTableSizes[hash]++;
			size++;
			SkipBracedSection(&p);
		}
		size += MAX_SHADERTEXT_HASH;

		hashMem = (char *)ri.Hunk_Alloc( size * sizeof(char *) );

		for (i = 0; i < MAX_SHADERTEXT_HASH; i++) {
			shaderTextHashTable[i] = (char **) hashMem;
			hashMem = ((char *) hashMem) + ((shaderTextHashTableSizes[i] + 1) * sizeof(char *));
		}

		memset(shaderTextHashTableSizes, 0, sizeof(shaderTextHashTableSizes));
		p = s_shaderText;
		// look for label
        COM_BeginParseSession ("ScanAndLoadShaderFiles2");
		while ( 1 ) {
			oldp = p;
			token = COM_ParseExt( &p, qtrue );
			if ( token[0] == 0 ) {
				break;
			}

			hash = generateHashValue(token, MAX_SHADERTEXT_HASH);
			shaderTextHashTable[hash][shaderTextHashTableSizes[hash]++] = oldp;

			SkipBracedSection(&p);
		}
	#endif		
	}
}


/*
textures/sfx/dclogo
{
	qer_editorimage textures/gothic_floor/largerblock3b.tga
	nomipmaps

	
	{
		map textures/base_floor/clangdark.tga
		rgbGen identity
		tcmod scale 4 4
	}

	{
		map $lightmap
		rgbGen identity
		blendfunc gl_dst_color gl_zero
	}
	
	
	{
		clampmap textures/effects/dreamcast-logo2.tga
		blendfunc gl_one gl_one
		tcmod rotate -75
		rgbGen wave sin .75 .25 0 .5
	}

	//	END
}
*/
static bool CheckForFilenameArg(const char * &psSearchPos, const char * psKeyword)
{
	const char * psSearchResult = strstr(psSearchPos,psKeyword);

	if (psSearchResult && isspace(psSearchResult[strlen(psKeyword)]))
	{
		psSearchResult += strlen(psKeyword);
		while (*psSearchResult && isspace(*psSearchResult)) psSearchResult++;

		if (strlen(psSearchResult) && *psSearchResult != '$' && ((psSearchResult>psSearchPos)?isspace(psSearchResult[-1]):1) )	// note error-check on -1 index
		{
			psSearchPos = psSearchResult;
			return true;
		}
	}

	if (psSearchResult)
	{
		// skip to line end...
		//
		while (*psSearchResult++ != '\n'){}
		psSearchResult++;
		psSearchPos = psSearchResult;
	}
	else
	{
		psSearchPos = NULL;
	}
	return false;
}

// have a look at the shader text pointed at by the supplied arg and try and work out
//	a suitable image name.
//
// First, see if there's an editorimage,
//	else check for a map with a valid texture (ie not '$lightmap' etc)
//  else check for a clampmap
//  else just return NULL, signifying that I'm stumped...
//
static const char * Shader_ExtractSuitableFilename(const char * psShaderText)
{
	char *psShaderTextEnd = (char*)psShaderText;
	SkipBracedSection (&psShaderTextEnd);
	
	int iShaderChars = psShaderTextEnd - psShaderText;

	// make a small string with just this shader in, for easier searching...
	//
	char *psThisShader	= (char *) malloc(iShaderChars+1);	// +1 for trailing zero
	char *psShaderSearch= psThisShader;	
	strncpy(psThisShader,psShaderText,iShaderChars);
	psThisShader[iShaderChars]='\0';

	const char * psAnswer = NULL;
	
	char *psShaderSearchTry = psShaderSearch;
	if (CheckForFilenameArg((const char *&)psShaderSearchTry, "qer_editorimage"))
	{
		psAnswer = psShaderSearchTry;
	}
	
	// these next ones I need to search for multiple times, else it hits things like "q3map_blah",
	//	doesn't find a space after "map", then gives up, so it should keep trying....
	//
	psShaderSearchTry = psShaderSearch;
	while (psShaderSearchTry && !psAnswer)
	{
		if (CheckForFilenameArg((const char *&)psShaderSearchTry, "map"))
		{
			psAnswer = psShaderSearchTry;
		}
	}

	if (!psAnswer)
	{
		psShaderSearchTry = psShaderSearch;

		while (psShaderSearchTry && !psAnswer)
		{
			if (CheckForFilenameArg((const char *&)psShaderSearchTry, "clampmap"))
			{
				psAnswer = psShaderSearchTry;
			}
		}
	}
	
	static char sReturnName[MAX_QPATH];
	if (psAnswer)
	{
		strncpy(sReturnName,psAnswer,sizeof(sReturnName));
		sReturnName[sizeof(sReturnName)-1]='\0';
		for (int i=0; i<sizeof(sReturnName)-1; i++)
		{
			if (sReturnName[i]=='\0')
				break;
			if (isspace(sReturnName[i]))
			{
				sReturnName[i] = '\0';
				break;
			}
		}
	}	

	free(psThisShader);
	return psAnswer?sReturnName:NULL;
}


// massively hacked-down version of original code, this simply takes an input char * local filename, then
//	modifies it to be either the name of the first MAP arg within the corresponding shader file, else leaves
//	it alone if not found
const char *R_FindShader( const char *psLocalMaterialName)
{
	char strippedName[MAX_QPATH];

	COM_StripExtension (psLocalMaterialName, strippedName );
	ToLower(strippedName);

	ShadersFoundAndFilesPicked_t::iterator it = ShadersFoundAndFilesPicked.find(strippedName);
	if (it != ShadersFoundAndFilesPicked.end())
	{
		return it->second.c_str();
	}
    
	//
	// attempt to define shader from an explicit parameter file
	//
	const char *shaderText = FindShaderInShaderText( strippedName );
	if ( shaderText ) 
	{
		const char * psReturnName = Shader_ExtractSuitableFilename(shaderText);
		if (psReturnName)
		{
			psLocalMaterialName = psReturnName;
		}
	}

	ShadersFoundAndFilesPicked[strippedName] = psLocalMaterialName;
	return psLocalMaterialName;	// return original name
}

////////////////// eof ////////////////



