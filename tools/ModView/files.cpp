#include "stdafx.h"
#include "files.h"

#include <algorithm>
#include <iostream>
#include <map>

#include "generic_stuff.h"
#include "stl.h"
#include "StringUtils.h"

#include <QtCore/QDir>

/*
=================================================================================

DIRECTORY SCANNING FUNCTIONS

=================================================================================
*/

namespace
{
#define	MAX_FOUND_FILES	0x1000

std::string joinPath(const std::string& s)
{
	return s;
}

template<typename... Ts>
std::string joinPath(const std::string& a, Ts... args)
{
	return a + QDir::separator().toLatin1() + joinPath(args...);
}

struct File
{
	std::string name;
};

struct FileSystem
{
	std::map<std::string, File> files;
};
}

bool InitFileSystem(const std::string& gameDataPath, const std::string& baseDir)
{
	const QDir gameDataDir(gameDataPath.c_str());
	if (!gameDataDir.exists())
	{
		return false;
	}

	const QDir basePath(joinPath(gameDataPath, baseDir).c_str());
	if (!basePath.exists())
	{
		return false;
	}

	const QString pk3Extension = "*.pk3";
	QStringList onlyPk3Extension;
	onlyPk3Extension.push_back(pk3Extension);

	const QStringList pk3Files = basePath.entryList(onlyPk3Extension, QDir::Files);
	for (const auto& file : pk3Files)
	{
		std::cout << file.toStdString() << '\n';
	}

	return true;
}

std::vector<std::string> Sys_ListFiles( const char *directory, const char *extension, bool wantsubs ) {
	int nfiles;
	std::vector<std::string> fileList;

	if (extension == nullptr) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if (extension[0] == '/' && extension[1] == '\0') {
		extension = "";
	}

	// search
	nfiles = 0;

	QDir root (directory);
	if ( !root.exists() )
	{
		return fileList;
	}

	QDir::Filters filters = QDir::Files;
	if ( wantsubs )
	{
		filters |= QDir::AllDirs;
	}

	QStringList nameFilters;
	QString starExtension = "*";
	starExtension += extension;
	nameFilters.push_back (starExtension);

	QStringList files = root.entryList (nameFilters, filters);
	nfiles = std::min (files.size(), MAX_FOUND_FILES - 1);
	
	fileList.resize (nfiles);
	
	for ( int i = 0; i < nfiles; i++ )
	{
		fileList[i] = files[i].toLatin1().constData();
	}

	return fileList;
}

#define BASEDIRNAME			"base" 
#define BASEDIRNAME_XMEN	"development" 

// kept as non-hungarian labels purely because these 2 are so common in our projects...
//
char		qdir[1024]={0};			// "S:\"
char		gamedir[1024]={0};		// "S:\\baseq3\\"

// unlike "normal" SetQdirFromPath() functions, I need to see if qdir has changed 
//	(ie by loading an EF model over an SOF model) which could mean that we're in a different dir, and so
//	should lose all cached textures etc...
//
bool bXMenPathHack = false;
bool bQ3RulesApply = true;
static bool SetQdirFromPath2( const char *path, const char *psBaseDir );
void SetQdirFromPath( const char *path )
{
	char prevQdir[1024];
	strcpy (prevQdir, qdir);

	bXMenPathHack = false;	// MUST be here

	if (SetQdirFromPath2(path,BASEDIRNAME))
	{
		bQ3RulesApply = true;
	}
	else
	{
		if (SetQdirFromPath2(path,BASEDIRNAME_XMEN))
		{
			bQ3RulesApply = false;
			bXMenPathHack = true;
		}
	}		

	if (Q_stricmp(prevQdir,qdir))
	{
		extern void Media_Delete(void);
		Media_Delete();
	}
}

static bool SetQdirFromPath2( const char *path2, const char *psBaseDir )
{
//	static bool bDone = false;
	
//	if (!bDone)
	{
//		bDone = true;

		char	temp[1024];
		char    path[1024];
		strcpy(temp,path2);
		strcpy(path,temp);
		const char	*c;
		const char *sep;
		int		len, count;
		
//		if (!(path[0] == '/' || path[0] == '\\' || path[1] == ':'))
//		{	// path is partial
//			Q_getwd (temp);
//			strcat (temp, path);
//			path = temp;
//		}
		
		ToLower(path);
		
		// search for "base" in path from the RIGHT hand side (and must have a [back]slash just before it)
		
		len = strlen(psBaseDir);
		for (c=path+strlen(path)-1 ; c != path ; c--)
		{
			int i;
			
			if (!Q_stricmpn (c, psBaseDir, len)
				&& 
				(*(c-1) == '/' || *(c-1) == '\\')	// would be more efficient to do this first, but only checking after a strncasecmp ok ensures no invalid pointer-1 access
				)
			{			
				sep = c + len;
				count = 1;
				while (*sep && *sep != '/' && *sep != '\\')
				{
					sep++;
					count++;
				}
				strncpy (gamedir, path, c+len+count-path);
				gamedir[c+len+count-path]='\0';
				for ( i = 0; i < (int)strlen( gamedir ); i++ )
				{
					if ( gamedir[i] == '\\' ) 
						gamedir[i] = '/';
				}
	//			qprintf ("gamedir: %s\n", gamedir);

				strncpy (qdir, path, c-path);
				qdir[c-path] = 0;
				for ( i = 0; i < (int)strlen( qdir ); i++ )
				{
					if ( qdir[i] == '\\' ) 
						qdir[i] = '/';
				}
	//			qprintf ("qdir: %s\n", qdir);

				return true;
			}
		}
	//	Error ("SetQdirFromPath: no '%s' in %s", BASEDIRNAME, path);
	}

	return false;
}

// takes (eg) "q:\quake\baseq3\textures\borg\name.tga"
//
//	and produces "textures/borg/name.tga"
//
void Filename_RemoveQUAKEBASE( std::string& string )
{
    std::replace(string.begin(), string.end(), '\\', '/');
    ToLower(string);
    string = Replace(string, gamedir, "");
}

bool FileExists (const char * psFilename)
{
	FILE *handle = fopen(psFilename, "r");
	if (!handle)
	{
		return false;
	}
	fclose (handle);
	return true;
}

long FileLen( const char * psFilename)
{
	FILE *handle = fopen(psFilename, "r");
	if (!handle)
	{
		return -1;
	}

	long l = filesize(handle);

	fclose (handle);
	return l;
}




// returns actual filename only, no path
//
char *Filename_WithoutPath(const char * psFilename)
{
	static char sString[MAX_QPATH];
/*
	const char * p = strrchr(psFilename,'\\');

	if (!p++)
	{
		p = strrchr(psFilename,'/');
		if (!p++)
			p=psFilename;
	}

	strcpy(sString,p);
*/

	const char * psCopyPos = psFilename;
	
	while (*psFilename)
	{
		if (*psFilename == '/' || *psFilename == '\\')
			psCopyPos = psFilename+1;
		psFilename++;
	}

	strcpy(sString,psCopyPos);

	return sString;

}


// returns (eg) "\dir\name" for "\dir\name.bmp"
//
char *Filename_WithoutExt(const char * psFilename)
{
	static char sString[MAX_QPATH];

	strcpy(sString,psFilename);

	char *p = strrchr(sString,'.');		
	char *p2= strrchr(sString,'\\');
	char *p3= strrchr(sString,'/');

	// special check, make sure the first suffix we found from the end wasn't just a directory suffix (eg on a path'd filename with no extension anyway)
	//
	if (p && 
		(p2==0 || (p2 && p>p2)) &&
		(p3==0 || (p3 && p>p3))
		)
		*p=0;	

	return sString;

}


// loses anything after the path (if any), (eg) "\dir\name.bmp" becomes "\dir"
//
char *Filename_PathOnly(const char * psFilename)
{
	static char sString[MAX_QPATH];

	strcpy(sString,psFilename);	
	
//	for (int i=0; i<strlen(sString); i++)
//	{
//		if (sString[i] == '/')
//			sString[i] = '\\';
//	}
		
	char *p1= strrchr(sString,'\\');
	char *p2= strrchr(sString,'/');
	char *p = (p1>p2)?p1:p2;
	if (p)
		*p=0;

	return sString;

}


// returns filename's extension only (including '.'), else returns original string if no '.' in it...
//
char *Filename_ExtOnly(const char * psFilename)
{
	static char sString[MAX_QPATH];
	const char * p = strrchr(psFilename,'.');

	if (!p)
		p=psFilename;

	strcpy(sString,p);

	return sString;

}

/* this piece copied out of on-line examples, it basically works as a
	similar function to Ffilelength(int handle), but it works with streams */
long filesize(FILE *handle)
{
   long curpos, length;

   curpos = ftell(handle);
   fseek(handle, 0L, SEEK_END);
   length = ftell(handle);
   fseek(handle, curpos, SEEK_SET);

   return length;
}


// returns -1 for error
int LoadFile (const char * psPathedFilename, void **bufferptr, int bReportErrors)
{
	FILE	*f;
	int    length;
	char    *buffer;

	f = fopen(psPathedFilename,"rb");
	if (f)
	{
		length = filesize(f);	
		buffer = static_cast<char *>(malloc (length + 1));
		buffer[length] = 0;
		int lread = fread (buffer, 1, length, f);	
		fclose (f);

		if (lread==length)
		{	
			*bufferptr = buffer;
			return length;
		}
		free(buffer);
	}

	extern bool gbInImageLoader;	// tacky, but keeps quieter errors
	if (bReportErrors && !gbInImageLoader)
	{
		ErrorBox(va("Error reading file %s!",psPathedFilename));
	}

	return -1;
}

// returns -1 for error
int SaveFile (const char * psPathedFilename, const void *pBuffer, int iSize)
{		
#ifdef USE_MFC
	extern void CreatePath (const char *path);
	CreatePath(psPathedFilename);
	FILE *f = fopen(psPathedFilename,"wb");
	if (f)
	{
		int iLength = fwrite(pBuffer, 1, iSize, f);
		fclose (f);

		if (iLength == iSize)
		{	
			return iLength;
		}

		ErrorBox(va("Error writing file \"%s\" (length %d), only %d bytes written!",psPathedFilename,iSize,iLength));
		return -1;
	}

	ErrorBox(va("Error opening file \"%s\" for writing!",psPathedFilename));
	
	return -1;
#else
	return iSize;
#endif
}