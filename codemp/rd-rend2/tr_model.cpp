/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_models.c -- model loading and caching

#include "tr_local.h"
#include <qcommon/sstring.h>

#define	LL(x) x=LittleLong(x)


static qboolean R_LoadMD3(model_t *mod, int lod, void *buffer, const char *modName);
static qboolean R_LoadMDR(model_t *mod, void *buffer, int filesize, const char *name );

/*
====================
R_RegisterMD3
====================
*/
qhandle_t R_RegisterMD3(const char *name, model_t *mod)
{
	unsigned	*buf;
	int			lod;
	int			ident;
	qboolean	loaded = qfalse;
	int			numLoaded;
	char filename[MAX_QPATH], namebuf[MAX_QPATH+20];
	char *fext, defex[] = "md3";

	numLoaded = 0;

	strcpy(filename, name);

	fext = strchr(filename, '.');
	if(!fext)
		fext = defex;
	else
	{
		*fext = '\0';
		fext++;
	}

	for (lod = MD3_MAX_LODS - 1 ; lod >= 0 ; lod--)
	{
		if(lod)
			Com_sprintf(namebuf, sizeof(namebuf), "%s_%d.%s", filename, lod, fext);
		else
			Com_sprintf(namebuf, sizeof(namebuf), "%s.%s", filename, fext);

		qboolean bAlreadyCached = qfalse;
		if( !CModelCache->LoadFile( namebuf, (void**)&buf, &bAlreadyCached ) )
			continue;

		ident = *(unsigned *)buf;
		if( !bAlreadyCached )
			ident = LittleLong(ident);
		
		switch(ident)
		{
			case MD3_IDENT:
				loaded = R_LoadMD3(mod, lod, buf, namebuf);
				break;
			case MDXA_IDENT:
				loaded = R_LoadMDXA(mod, buf, namebuf, bAlreadyCached);
				break;
			case MDXM_IDENT:
				loaded = R_LoadMDXM(mod, buf, name, bAlreadyCached);
				break;
			default:
				ri->Printf(PRINT_WARNING, "R_RegisterMD3: unknown ident for %s\n", name);
				break;
		}

		if(loaded)
		{
			mod->numLods++;
			numLoaded++;
		}
		else
			break;
	}

	if(numLoaded)
	{
		// duplicate into higher lod spots that weren't
		// loaded, in case the user changes r_lodbias on the fly
		for(lod--; lod >= 0; lod--)
		{
			mod->numLods++;
			mod->data.mdv[lod] = mod->data.mdv[lod + 1];
		}

		return mod->index;
	}

#ifdef _DEBUG
	ri->Printf(PRINT_WARNING,"R_RegisterMD3: couldn't load %s\n", name);
#endif

	mod->type = MOD_BAD;
	return 0;
}

/*
====================
R_RegisterMDR
====================
*/
qhandle_t R_RegisterMDR(const char *name, model_t *mod)
{
	union {
		unsigned *u;
		void *v;
	} buf;
	int	ident;
	qboolean loaded = qfalse;
	int filesize;

	filesize = ri->FS_ReadFile(name, (void **) &buf.v);
	if(!buf.u)
	{
		mod->type = MOD_BAD;
		return 0;
	}
	
	ident = LittleLong(*(unsigned *)buf.u);
	if(ident == MDR_IDENT)
		loaded = R_LoadMDR(mod, buf.u, filesize, name);

	ri->FS_FreeFile (buf.v);
	
	if(!loaded)
	{
		ri->Printf(PRINT_WARNING,"R_RegisterMDR: couldn't load mdr file %s\n", name);
		mod->type = MOD_BAD;
		return 0;
	}
	
	return mod->index;
}

/*
====================
R_RegisterIQM
====================
*/
qhandle_t R_RegisterIQM(const char *name, model_t *mod)
{
	union {
		unsigned *u;
		void *v;
	} buf;
	qboolean loaded = qfalse;
	int filesize;

	filesize = ri->FS_ReadFile(name, (void **) &buf.v);
	if(!buf.u)
	{
		mod->type = MOD_BAD;
		return 0;
	}
	
	loaded = R_LoadIQM(mod, buf.u, filesize, name);

	ri->FS_FreeFile (buf.v);
	
	if(!loaded)
	{
		ri->Printf(PRINT_WARNING,"R_RegisterIQM: couldn't load iqm file %s\n", name);
		mod->type = MOD_BAD;
		return 0;
	}
	
	return mod->index;
}


typedef struct
{
	char *ext;
	qhandle_t (*ModelLoader)( const char *, model_t * );
} modelExtToLoaderMap_t;

// Note that the ordering indicates the order of preference used
// when there are multiple models of different formats available
static modelExtToLoaderMap_t modelLoaders[ ] =
{
	{ "iqm", R_RegisterIQM },
	{ "mdr", R_RegisterMDR },
	{ "md3", R_RegisterMD3 },
	/* 
	Ghoul 2 Insert Start
	*/
	{ "glm", R_RegisterMD3 },
	{ "gla", R_RegisterMD3 },
	/*
	Ghoul 2 Insert End
	*/
};

static int numModelLoaders = ARRAY_LEN(modelLoaders);

/*
** R_GetModelByHandle
*/
model_t	*R_GetModelByHandle( qhandle_t index ) {
	model_t		*mod;

	// out of range gets the defualt model
	if ( index < 1 || index >= tr.numModels ) {
		return tr.models[0];
	}

	mod = tr.models[index];

	return mod;
}

//===============================================================================

/*
** R_AllocModel
*/
model_t *R_AllocModel( void ) {
	model_t		*mod;

	if ( tr.numModels == MAX_MOD_KNOWN ) {
		return NULL;
	}

	mod = (model_t *)ri->Hunk_Alloc( sizeof( *tr.models[tr.numModels] ), h_low );
	mod->index = tr.numModels;
	tr.models[tr.numModels] = mod;
	tr.numModels++;

	return mod;
}

/*
====================
RE_RegisterModel

Loads in a model for the given name

Zero will be returned if the model fails to load.
An entry will be retained for failed models as an
optimization to prevent disk rescanning if they are
asked for again.
====================
*/
qhandle_t RE_RegisterModel( const char *name ) {
	model_t		*mod;
	qhandle_t	hModel;
	qboolean	orgNameFailed = qfalse;
	int			orgLoader = -1;
	int			i;
	char		localName[ MAX_QPATH ];
	const char	*ext;
	char		altName[ MAX_QPATH ];

	if ( !name || !name[0] ) {
		ri->Printf( PRINT_ALL, "RE_RegisterModel: NULL name\n" );
		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		ri->Printf( PRINT_ALL, "Model name exceeds MAX_QPATH\n" );
		return 0;
	}

	// search the currently loaded models
	if( ( hModel = CModelCache->SearchLoaded( name ) ) != -1 )
		return hModel;

	if ( name[0] == '*' )
	{
		if ( strcmp (name, "*default.gla") != 0 )
		{
			return 0;
		}
	}

	if( name[0] == '#' )
	{
		// TODO: BSP models
		return 0;
	}

	// allocate a new model_t
	if ( ( mod = R_AllocModel() ) == NULL ) {
		ri->Printf( PRINT_WARNING, "RE_RegisterModel: R_AllocModel() failed for '%s'\n", name);
		return 0;
	}

	// only set the name after the model has been successfully loaded
	Q_strncpyz( mod->name, name, sizeof( mod->name ) );

	R_IssuePendingRenderCommands();

	mod->type = MOD_BAD;
	mod->numLods = 0;

	//
	// load the files
	//
	Q_strncpyz( localName, name, MAX_QPATH );

	ext = COM_GetExtension( localName );

	if( *ext )
	{
		// Look for the correct loader and use it
		for( i = 0; i < numModelLoaders; i++ )
		{
			if( !Q_stricmp( ext, modelLoaders[ i ].ext ) )
			{
				// Load
				hModel = modelLoaders[ i ].ModelLoader( localName, mod );
				break;
			}
		}

		// A loader was found
		if( i < numModelLoaders )
		{
			if( !hModel )
			{
				// Loader failed, most likely because the file isn't there;
				// try again without the extension
				orgNameFailed = qtrue;
				orgLoader = i;
				COM_StripExtension( name, localName, MAX_QPATH );
			}
			else
			{
				// Something loaded
				CModelCache->InsertLoaded( name, hModel );
				return mod->index;
			}
		}
	}

	// Try and find a suitable match using all
	// the model formats supported
	for( i = 0; i < numModelLoaders; i++ )
	{
		if (i == orgLoader)
			continue;

		Com_sprintf( altName, sizeof (altName), "%s.%s", localName, modelLoaders[ i ].ext );

		// Load
		hModel = modelLoaders[ i ].ModelLoader( altName, mod );

		if( hModel )
		{
			if( orgNameFailed )
			{
				ri->Printf( PRINT_DEVELOPER, "WARNING: %s not present, using %s instead\n",
						name, altName );
			}

			break;
		}
	}

	CModelCache->InsertLoaded( name, hModel );
	return hModel;
}

//rww - Please forgive me for all of the below. Feel free to destroy it and replace it with something better.
//You obviously can't touch anything relating to shaders or ri-> functions here in case a dedicated
//server is running, which is the entire point of having these seperate functions. If anything major
//is changed in the non-server-only versions of these functions it would be wise to incorporate it
//here as well.

/*
=================
R_LoadMDXA_Server - load a Ghoul 2 animation file
=================
*/
qboolean R_LoadMDXA_Server( model_t *mod, void *buffer, const char *mod_name, qboolean &bAlreadyCached ) {

	mdxaHeader_t		*pinmodel, *mdxa;
	int					version;
	int					size;

 	pinmodel = (mdxaHeader_t *)buffer;
	//
	// read some fields from the binary, but only LittleLong() them when we know this wasn't an already-cached model...
	//	
	version = (pinmodel->version);
	size	= (pinmodel->ofsEnd);

	if (!bAlreadyCached)
	{
		LL(version);
		LL(size);
	}
	
	if (version != MDXA_VERSION) {
		return qfalse;
	}

	mod->type		= MOD_MDXA;
	mod->dataSize  += size;

	qboolean bAlreadyFound = qfalse;
	mdxa = (mdxaHeader_t*)CModelCache->Allocate( size, buffer, mod_name, &bAlreadyFound, TAG_MODEL_GLA );
	mod->data.gla = mdxa;

	assert(bAlreadyCached == bAlreadyFound);	// I should probably eliminate 'bAlreadyFound', but wtf?

	if (!bAlreadyFound)
	{
		// horrible new hackery, if !bAlreadyFound then we've just done a tag-morph, so we need to set the 
		//	bool reference passed into this function to true, to tell the caller NOT to do an ri->FS_Freefile since
		//	we've hijacked that memory block...
		//
		// Aaaargh. Kill me now...
		//
		bAlreadyCached = qtrue;
		assert( mdxa == buffer );
//		memcpy( mdxa, buffer, size );	// and don't do this now, since it's the same thing

		LL(mdxa->ident);
		LL(mdxa->version);
		LL(mdxa->numFrames);
		LL(mdxa->numBones);
		LL(mdxa->ofsFrames);
		LL(mdxa->ofsEnd);
	}

 	if ( mdxa->numFrames < 1 ) {
		return qfalse;
	}

	if (bAlreadyFound)
	{
		return qtrue;	// All done, stop here, do not LittleLong() etc. Do not pass go...
	}

	return qtrue;
}

/*
=================
R_LoadMDXM_Server - load a Ghoul 2 Mesh file
=================
*/
qboolean R_LoadMDXM_Server( model_t *mod, void *buffer, const char *mod_name, qboolean &bAlreadyCached ) {
	int					i,l, j;
	mdxmHeader_t		*pinmodel, *mdxm;
	mdxmLOD_t			*lod;
	mdxmSurface_t		*surf;
	int					version;
	int					size;
	//shader_t			*sh;
	mdxmSurfHierarchy_t	*surfInfo;
    
	pinmodel= (mdxmHeader_t *)buffer;
	//
	// read some fields from the binary, but only LittleLong() them when we know this wasn't an already-cached model...
	//	
	version = (pinmodel->version);
	size	= (pinmodel->ofsEnd);

	if (!bAlreadyCached)
	{
		LL(version);
		LL(size);
	}

	if (version != MDXM_VERSION) {
		return qfalse;
	}

	mod->type	   = MOD_MDXM;
	mod->dataSize += size;	
	
	qboolean bAlreadyFound = qfalse;
	mdxm = (mdxmHeader_t*)CModelCache->Allocate( size, buffer, mod_name, &bAlreadyFound, TAG_MODEL_GLM );
	mod->data.glm = (mdxmData_t *)ri->Hunk_Alloc (sizeof (mdxmData_t), h_low);
	mod->data.glm->header = mdxm;

	assert(bAlreadyCached == bAlreadyFound);	// I should probably eliminate 'bAlreadyFound', but wtf?

	if (!bAlreadyFound)
	{
		// horrible new hackery, if !bAlreadyFound then we've just done a tag-morph, so we need to set the 
		//	bool reference passed into this function to true, to tell the caller NOT to do an ri->FS_Freefile since
		//	we've hijacked that memory block...
		//
		// Aaaargh. Kill me now...
		//
		bAlreadyCached = qtrue;
		assert( mdxm == buffer );
//		memcpy( mdxm, buffer, size );	// and don't do this now, since it's the same thing

		LL(mdxm->ident);
		LL(mdxm->version);
		LL(mdxm->numLODs);
		LL(mdxm->ofsLODs);
		LL(mdxm->numSurfaces);
		LL(mdxm->ofsSurfHierarchy);
		LL(mdxm->ofsEnd);
	}
		
	// first up, go load in the animation file we need that has the skeletal animation info for this model
	mdxm->animIndex = RE_RegisterServerModel(va ("%s.gla",mdxm->animName));
	if (!mdxm->animIndex) 
	{
		return qfalse;
	}

	mod->numLods = mdxm->numLODs -1 ;	//copy this up to the model for ease of use - it wil get inced after this.

	if (bAlreadyFound)
	{
		return qtrue;	// All done. Stop, go no further, do not LittleLong(), do not pass Go...
	}

	surfInfo = (mdxmSurfHierarchy_t *)( (byte *)mdxm + mdxm->ofsSurfHierarchy);
 	for ( i = 0 ; i < mdxm->numSurfaces ; i++) 
	{
		LL(surfInfo->numChildren);
		LL(surfInfo->parentIndex);

		// do all the children indexs
		for (j=0; j<surfInfo->numChildren; j++)
		{
			LL(surfInfo->childIndexes[j]);
		}

		// We will not be using shaders on the server.
		//sh = 0;
		// insert it in the surface list
		
		surfInfo->shaderIndex = 0;

		CModelCache->StoreShaderRequest(mod_name, &surfInfo->shader[0], &surfInfo->shaderIndex);

		// find the next surface
		surfInfo = (mdxmSurfHierarchy_t *)( (byte *)surfInfo + (intptr_t)( &((mdxmSurfHierarchy_t *)0)->childIndexes[ surfInfo->numChildren ] ));
  	}
	
	// swap all the LOD's	(we need to do the middle part of this even for intel, because of shader reg and err-check)
	lod = (mdxmLOD_t *) ( (byte *)mdxm + mdxm->ofsLODs );
	for ( l = 0 ; l < mdxm->numLODs ; l++)
	{
		int	triCount = 0;

		LL(lod->ofsEnd);
		// swap all the surfaces
		surf = (mdxmSurface_t *) ( (byte *)lod + sizeof (mdxmLOD_t) + (mdxm->numSurfaces * sizeof(mdxmLODSurfOffset_t)) );
		for ( i = 0 ; i < mdxm->numSurfaces ; i++) 
		{
			LL(surf->numTriangles);
			LL(surf->ofsTriangles);
			LL(surf->numVerts);
			LL(surf->ofsVerts);
			LL(surf->ofsEnd);
			LL(surf->ofsHeader);
			LL(surf->numBoneReferences);
			LL(surf->ofsBoneReferences);
//			LL(surf->maxVertBoneWeights);

			triCount += surf->numTriangles;
										
			if ( surf->numVerts > SHADER_MAX_VERTEXES ) {
				return qfalse;
			}
			if ( surf->numTriangles*3 > SHADER_MAX_INDEXES ) {
				return qfalse;
			}
		
			// change to surface identifier
			surf->ident = SF_MDX;

			// register the shaders

			// find the next surface
			surf = (mdxmSurface_t *)( (byte *)surf + surf->ofsEnd );
		}

		// find the next LOD
		lod = (mdxmLOD_t *)( (byte *)lod + lod->ofsEnd );
	}

	return qtrue;
}

/*
====================
R_RegisterMDX_Server
====================
*/
qhandle_t R_RegisterMDX_Server(const char *name, model_t *mod)
{
	unsigned	*buf;
	int			lod;
	int			ident;
	qboolean	loaded = qfalse;
	int			numLoaded;
	char filename[MAX_QPATH], namebuf[MAX_QPATH+20];
	char *fext, defex[] = "md3";

	numLoaded = 0;

	strcpy(filename, name);

	fext = strchr(filename, '.');
	if(!fext)
		fext = defex;
	else
	{
		*fext = '\0';
		fext++;
	}

	for (lod = MD3_MAX_LODS - 1 ; lod >= 0 ; lod--)
	{
		if(lod)
			Com_sprintf(namebuf, sizeof(namebuf), "%s_%d.%s", filename, lod, fext);
		else
			Com_sprintf(namebuf, sizeof(namebuf), "%s.%s", filename, fext);

		qboolean bAlreadyCached = qfalse;
		if( !CModelCache->LoadFile( namebuf, (void**)&buf, &bAlreadyCached ) )
			continue;

		ident = *(unsigned *)buf;
		if( !bAlreadyCached )
			ident = LittleLong(ident);
		
		switch(ident)
		{
			case MDXA_IDENT:
				loaded = R_LoadMDXA_Server(mod, buf, namebuf, bAlreadyCached);
				break;
			case MDXM_IDENT:
				loaded = R_LoadMDXM_Server(mod, buf, namebuf, bAlreadyCached);
				break;
			default:
				//ri->Printf(PRINT_WARNING, "R_RegisterMDX_Server: unknown ident for %s\n", name);
				break;
		}

		if(loaded)
		{
			mod->numLods++;
			numLoaded++;
		}
		else
			break;
	}

	if(numLoaded)
	{
		// duplicate into higher lod spots that weren't
		// loaded, in case the user changes r_lodbias on the fly
		for(lod--; lod >= 0; lod--)
		{
			mod->numLods++;
			mod->data.mdv[lod] = mod->data.mdv[lod + 1];
		}

		return mod->index;
	}

/*#ifdef _DEBUG
	ri->Printf(PRINT_WARNING,"R_RegisterMDX_Server: couldn't load %s\n", name);
#endif*/

	mod->type = MOD_BAD;
	return 0;
}

// Note that the ordering indicates the order of preference used
// when there are multiple models of different formats available
static modelExtToLoaderMap_t serverModelLoaders[ ] =
{
	/* 
	Ghoul 2 Insert Start
	*/
	{ "glm", R_RegisterMDX_Server },
	{ "gla", R_RegisterMDX_Server },
	/*
	Ghoul 2 Insert End
	*/
};

static int numServerModelLoaders = ARRAY_LEN(serverModelLoaders);

qhandle_t RE_RegisterServerModel( const char *name ) {
	model_t		*mod;
	qhandle_t	hModel;
	int			i;
	char		localName[ MAX_QPATH ];
	const char	*ext;

	if (!r_noServerGhoul2)
	{ //keep it from choking when it gets to these checks in the g2 code. Registering all r_ cvars for the server would be a Bad Thing though.
		r_noServerGhoul2 = ri->Cvar_Get( "r_noserverghoul2", "0", 0);
	}

	if ( !name || !name[0] ) {
		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		return 0;
	}

	// search the currently loaded models
	if( ( hModel = CModelCache->SearchLoaded( name ) ) != -1 )
		return hModel;

	if ( name[0] == '*' )
	{
		if ( strcmp (name, "*default.gla") != 0 )
		{
			return 0;
		}
	}

	// allocate a new model_t
	if ( ( mod = R_AllocModel() ) == NULL ) {
		ri->Printf( PRINT_WARNING, "RE_RegisterModel: R_AllocModel() failed for '%s'\n", name);
		return 0;
	}

	// only set the name after the model has been successfully loaded
	Q_strncpyz( mod->name, name, sizeof( mod->name ) );

	R_IssuePendingRenderCommands();

	mod->type = MOD_BAD;
	mod->numLods = 0;

	//
	// load the files
	//
	Q_strncpyz( localName, name, MAX_QPATH );

	ext = COM_GetExtension( localName );

	if( *ext )
	{
		// Look for the correct loader and use it
		for( i = 0; i < numServerModelLoaders; i++ )
		{
			if( !Q_stricmp( ext, serverModelLoaders[ i ].ext ) )
			{
				// Load
				hModel = serverModelLoaders[ i ].ModelLoader( localName, mod );
				break;
			}
		}

		// A loader was found
		if( i < numServerModelLoaders )
		{
			if( hModel )
			{
				// Something loaded
				CModelCache->InsertLoaded( name, hModel );
				return mod->index;
			}
		}
	}

	CModelCache->InsertLoaded( name, hModel );
	return hModel;
}

/*
=================
R_LoadMD3
=================
*/
static qboolean R_LoadMD3(model_t * mod, int lod, void *buffer, const char *modName)
{
	int             f, i, j;

	md3Header_t    *md3Model;
	md3Frame_t     *md3Frame;
	md3Surface_t   *md3Surf;
	md3Shader_t    *md3Shader;
	md3Triangle_t  *md3Tri;
	md3St_t        *md3st;
	md3XyzNormal_t *md3xyz;
	md3Tag_t       *md3Tag;

	mdvModel_t     *mdvModel;
	mdvFrame_t     *frame;
	mdvSurface_t   *surf;//, *surface;
	int            *shaderIndex;
	glIndex_t	   *tri;
	mdvVertex_t    *v;
	mdvSt_t        *st;
	mdvTag_t       *tag;
	mdvTagName_t   *tagName;

	int             version;
	int             size;

	md3Model = (md3Header_t *) buffer;

	version = LittleLong(md3Model->version);
	if(version != MD3_VERSION)
	{
		ri->Printf(PRINT_WARNING, "R_LoadMD3: %s has wrong version (%i should be %i)\n", modName, version, MD3_VERSION);
		return qfalse;
	}

	mod->type = MOD_MESH;
	size = LittleLong(md3Model->ofsEnd);
	mod->dataSize += size;
	//mdvModel = mod->mdv[lod] = (mdvModel_t *)ri->Hunk_Alloc(sizeof(mdvModel_t), h_low);
	qboolean bAlreadyFound = qfalse;
	mdvModel = mod->data.mdv[lod] = (mdvModel_t *)CModelCache->Allocate( size, buffer, modName, &bAlreadyFound, TAG_MODEL_MD3 );

//  Com_Memcpy(mod->md3[lod], buffer, LittleLong(md3Model->ofsEnd));
	if( !bAlreadyFound )
	{	// HACK
		LL(md3Model->ident);
		LL(md3Model->version);
		LL(md3Model->numFrames);
		LL(md3Model->numTags);
		LL(md3Model->numSurfaces);
		LL(md3Model->ofsFrames);
		LL(md3Model->ofsTags);
		LL(md3Model->ofsSurfaces);
		LL(md3Model->ofsEnd);
	}
	else
	{
		CModelCache->AllocateShaders( modName );
	}

	if(md3Model->numFrames < 1)
	{
		ri->Printf(PRINT_WARNING, "R_LoadMD3: %s has no frames\n", modName);
		return qfalse;
	}

	// swap all the frames
	mdvModel->numFrames = md3Model->numFrames;
	mdvModel->frames = frame = (mdvFrame_t *)ri->Hunk_Alloc(sizeof(*frame) * md3Model->numFrames, h_low);

	md3Frame = (md3Frame_t *) ((byte *) md3Model + md3Model->ofsFrames);
	for(i = 0; i < md3Model->numFrames; i++, frame++, md3Frame++)
	{
		frame->radius = LittleFloat(md3Frame->radius);
		for(j = 0; j < 3; j++)
		{
			frame->bounds[0][j] = LittleFloat(md3Frame->bounds[0][j]);
			frame->bounds[1][j] = LittleFloat(md3Frame->bounds[1][j]);
			frame->localOrigin[j] = LittleFloat(md3Frame->localOrigin[j]);
		}
	}

	// swap all the tags
	mdvModel->numTags = md3Model->numTags;
	mdvModel->tags = tag = (mdvTag_t *)ri->Hunk_Alloc(sizeof(*tag) * (md3Model->numTags * md3Model->numFrames), h_low);

	md3Tag = (md3Tag_t *) ((byte *) md3Model + md3Model->ofsTags);
	for(i = 0; i < md3Model->numTags * md3Model->numFrames; i++, tag++, md3Tag++)
	{
		for(j = 0; j < 3; j++)
		{
			tag->origin[j] = LittleFloat(md3Tag->origin[j]);
			tag->axis[0][j] = LittleFloat(md3Tag->axis[0][j]);
			tag->axis[1][j] = LittleFloat(md3Tag->axis[1][j]);
			tag->axis[2][j] = LittleFloat(md3Tag->axis[2][j]);
		}
	}


	mdvModel->tagNames = tagName = (mdvTagName_t *)ri->Hunk_Alloc(sizeof(*tagName) * (md3Model->numTags), h_low);

	md3Tag = (md3Tag_t *) ((byte *) md3Model + md3Model->ofsTags);
	for(i = 0; i < md3Model->numTags; i++, tagName++, md3Tag++)
	{
		Q_strncpyz(tagName->name, md3Tag->name, sizeof(tagName->name));
	}

	// swap all the surfaces
	mdvModel->numSurfaces = md3Model->numSurfaces;
	mdvModel->surfaces = surf = (mdvSurface_t *)ri->Hunk_Alloc(sizeof(*surf) * md3Model->numSurfaces, h_low);

	md3Surf = (md3Surface_t *) ((byte *) md3Model + md3Model->ofsSurfaces);
	for(i = 0; i < md3Model->numSurfaces; i++)
	{
		LL(md3Surf->ident);
		LL(md3Surf->flags);
		LL(md3Surf->numFrames);
		LL(md3Surf->numShaders);
		LL(md3Surf->numTriangles);
		LL(md3Surf->ofsTriangles);
		LL(md3Surf->numVerts);
		LL(md3Surf->ofsShaders);
		LL(md3Surf->ofsSt);
		LL(md3Surf->ofsXyzNormals);
		LL(md3Surf->ofsEnd);

		if(md3Surf->numVerts >= SHADER_MAX_VERTEXES)
		{
			ri->Printf(PRINT_WARNING, "R_LoadMD3: %s has more than %i verts on %s (%i).\n",
				modName, SHADER_MAX_VERTEXES - 1, md3Surf->name[0] ? md3Surf->name : "a surface",
				md3Surf->numVerts );
			return qfalse;
		}
		if(md3Surf->numTriangles * 3 >= SHADER_MAX_INDEXES)
		{
			ri->Printf(PRINT_WARNING, "R_LoadMD3: %s has more than %i triangles on %s (%i).\n",
				modName, ( SHADER_MAX_INDEXES / 3 ) - 1, md3Surf->name[0] ? md3Surf->name : "a surface",
				md3Surf->numTriangles );
			return qfalse;
		}

		// change to surface identifier
		surf->surfaceType = SF_MDV;

		// give pointer to model for Tess_SurfaceMDX
		surf->model = mdvModel;

		// copy surface name
		Q_strncpyz(surf->name, md3Surf->name, sizeof(surf->name));

		// lowercase the surface name so skin compares are faster
		Q_strlwr(surf->name);

		// strip off a trailing _1 or _2
		// this is a crutch for q3data being a mess
		j = strlen(surf->name);
		if(j > 2 && surf->name[j - 2] == '_')
		{
			surf->name[j - 2] = 0;
		}

		// register the shaders
		surf->numShaderIndexes = md3Surf->numShaders;
		surf->shaderIndexes = shaderIndex = (int *)ri->Hunk_Alloc(sizeof(*shaderIndex) * md3Surf->numShaders, h_low);

		md3Shader = (md3Shader_t *) ((byte *) md3Surf + md3Surf->ofsShaders);
		for(j = 0; j < md3Surf->numShaders; j++, shaderIndex++, md3Shader++)
		{
			shader_t       *sh;

			sh = R_FindShader(md3Shader->name, lightmapsNone, stylesDefault, qtrue);
			if(sh->defaultShader)
			{
				*shaderIndex = 0;
			}
			else
			{
				*shaderIndex = sh->index;
			}
		}

		// swap all the triangles
		surf->numIndexes = md3Surf->numTriangles * 3;
		surf->indexes = tri = (glIndex_t *)ri->Hunk_Alloc(sizeof(*tri) * 3 * md3Surf->numTriangles, h_low);

		md3Tri = (md3Triangle_t *) ((byte *) md3Surf + md3Surf->ofsTriangles);
		for(j = 0; j < md3Surf->numTriangles; j++, tri += 3, md3Tri++)
		{
			tri[0] = LittleLong(md3Tri->indexes[0]);
			tri[1] = LittleLong(md3Tri->indexes[1]);
			tri[2] = LittleLong(md3Tri->indexes[2]);
		}

		// swap all the XyzNormals
		surf->numVerts = md3Surf->numVerts;
		surf->verts = v = (mdvVertex_t *)ri->Hunk_Alloc(sizeof(*v) * (md3Surf->numVerts * md3Surf->numFrames), h_low);

		md3xyz = (md3XyzNormal_t *) ((byte *) md3Surf + md3Surf->ofsXyzNormals);
		for(j = 0; j < md3Surf->numVerts * md3Surf->numFrames; j++, md3xyz++, v++)
		{
			unsigned lat, lng;
			unsigned short normal;

			v->xyz[0] = LittleShort(md3xyz->xyz[0]) * MD3_XYZ_SCALE;
			v->xyz[1] = LittleShort(md3xyz->xyz[1]) * MD3_XYZ_SCALE;
			v->xyz[2] = LittleShort(md3xyz->xyz[2]) * MD3_XYZ_SCALE;

			normal = LittleShort(md3xyz->normal);

			lat = ( normal >> 8 ) & 0xff;
			lng = ( normal & 0xff );
			lat *= (FUNCTABLE_SIZE/256);
			lng *= (FUNCTABLE_SIZE/256);

			// decode X as cos( lat ) * sin( long )
			// decode Y as sin( lat ) * sin( long )
			// decode Z as cos( long )

			v->normal[0] = tr.sinTable[(lat+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK] * tr.sinTable[lng];
			v->normal[1] = tr.sinTable[lat] * tr.sinTable[lng];
			v->normal[2] = tr.sinTable[(lng+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK];
		}

		// swap all the ST
		surf->st = st = (mdvSt_t *)ri->Hunk_Alloc(sizeof(*st) * md3Surf->numVerts, h_low);

		md3st = (md3St_t *) ((byte *) md3Surf + md3Surf->ofsSt);
		for(j = 0; j < md3Surf->numVerts; j++, md3st++, st++)
		{
			st->st[0] = LittleFloat(md3st->st[0]);
			st->st[1] = LittleFloat(md3st->st[1]);
		}

		// calc tangent spaces
		{
			for(j = 0, v = surf->verts; j < (surf->numVerts * mdvModel->numFrames); j++, v++)
			{
				VectorClear(v->tangent);
				VectorClear(v->bitangent);
			}

			for(f = 0; f < mdvModel->numFrames; f++)
			{
				for(j = 0, tri = surf->indexes; j < surf->numIndexes; j += 3, tri += 3)
				{
					vec3_t sdir, tdir;
					const float *v0, *v1, *v2, *t0, *t1, *t2;
					glIndex_t index0, index1, index2;

					index0 = surf->numVerts * f + tri[0];
					index1 = surf->numVerts * f + tri[1];
					index2 = surf->numVerts * f + tri[2];

					v0 = surf->verts[index0].xyz;
					v1 = surf->verts[index1].xyz;
					v2 = surf->verts[index2].xyz;

					t0 = surf->st[tri[0]].st;
					t1 = surf->st[tri[1]].st;
					t2 = surf->st[tri[2]].st;

					R_CalcTexDirs(sdir, tdir, v0, v1, v2, t0, t1, t2);

					VectorAdd(sdir, surf->verts[index0].tangent,   surf->verts[index0].tangent);
					VectorAdd(sdir, surf->verts[index1].tangent,   surf->verts[index1].tangent);
					VectorAdd(sdir, surf->verts[index2].tangent,   surf->verts[index2].tangent);
					VectorAdd(tdir, surf->verts[index0].bitangent, surf->verts[index0].bitangent);
					VectorAdd(tdir, surf->verts[index1].bitangent, surf->verts[index1].bitangent);
					VectorAdd(tdir, surf->verts[index2].bitangent, surf->verts[index2].bitangent);
				}
			}

			for(j = 0, v = surf->verts; j < (surf->numVerts * mdvModel->numFrames); j++, v++)
			{
				vec3_t sdir, tdir;

				VectorCopy(v->tangent,   sdir);
				VectorCopy(v->bitangent, tdir);

				VectorNormalize(sdir);
				VectorNormalize(tdir);

				R_CalcTbnFromNormalAndTexDirs(v->tangent, v->bitangent, v->normal, sdir, tdir);
			}
		}

		// find the next surface
		md3Surf = (md3Surface_t *) ((byte *) md3Surf + md3Surf->ofsEnd);
		surf++;
	}

	{
		srfVBOMDVMesh_t *vboSurf;

		mdvModel->numVBOSurfaces = mdvModel->numSurfaces;
		mdvModel->vboSurfaces = (srfVBOMDVMesh_t *)ri->Hunk_Alloc(sizeof(*mdvModel->vboSurfaces) * mdvModel->numSurfaces, h_low);

		vboSurf = mdvModel->vboSurfaces;
		surf = mdvModel->surfaces;
		for (i = 0; i < mdvModel->numSurfaces; i++, vboSurf++, surf++)
		{
			vec3_t *verts;
			vec2_t *texcoords;
			uint32_t *normals;
			uint32_t *tangents;

			byte *data;
			int dataSize;

			int ofs_xyz, ofs_normal, ofs_st;
			int ofs_tangent;

			dataSize = 0;

			ofs_xyz = dataSize;
			dataSize += surf->numVerts * mdvModel->numFrames * sizeof(*verts);

			ofs_normal = dataSize;
			dataSize += surf->numVerts * mdvModel->numFrames * sizeof(*normals);

			ofs_tangent = dataSize;
			dataSize += surf->numVerts * mdvModel->numFrames * sizeof(*tangents);

			ofs_st = dataSize;
			dataSize += surf->numVerts * sizeof(*texcoords);

			data = (byte *)Z_Malloc(dataSize, TAG_MODEL_MD3);

			verts =      (vec3_t *)(data + ofs_xyz);
			normals =    (uint32_t *)(data + ofs_normal);
			tangents =   (uint32_t *)(data + ofs_tangent);
			texcoords =  (vec2_t *)(data + ofs_st);
		
			v = surf->verts;
			for ( j = 0; j < surf->numVerts * mdvModel->numFrames ; j++, v++ )
			{
				vec3_t nxt;
				vec4_t tangent;

				VectorCopy(v->xyz,       verts[j]);

				normals[j] = R_VboPackNormal(v->normal);
				CrossProduct(v->normal, v->tangent, nxt);
				VectorCopy(v->tangent, tangent);
				tangent[3] = (DotProduct(nxt, v->bitangent) < 0.0f) ? -1.0f : 1.0f;

				tangents[j] = R_VboPackTangent(tangent);
			}

			st = surf->st;
			for ( j = 0 ; j < surf->numVerts ; j++, st++ ) {
				texcoords[j][0] = st->st[0];
				texcoords[j][1] = st->st[1];
			}

			vboSurf->surfaceType = SF_VBO_MDVMESH;
			vboSurf->mdvModel = mdvModel;
			vboSurf->mdvSurface = surf;
			vboSurf->numIndexes = surf->numIndexes;
			vboSurf->numVerts = surf->numVerts;
			
			vboSurf->minIndex = 0;
			vboSurf->maxIndex = surf->numVerts;

			vboSurf->vbo = R_CreateVBO(data, dataSize, VBO_USAGE_STATIC);

			vboSurf->vbo->offsets[ATTR_INDEX_POSITION]  = ofs_xyz;
			vboSurf->vbo->offsets[ATTR_INDEX_NORMAL]    = ofs_normal;
			vboSurf->vbo->offsets[ATTR_INDEX_TEXCOORD0] = ofs_st;
			vboSurf->vbo->offsets[ATTR_INDEX_TANGENT]   = ofs_tangent;

			vboSurf->vbo->strides[ATTR_INDEX_POSITION]  = sizeof(*verts);
			vboSurf->vbo->strides[ATTR_INDEX_NORMAL]    = sizeof(*normals);
			vboSurf->vbo->strides[ATTR_INDEX_TEXCOORD0] = sizeof(*st);
			vboSurf->vbo->strides[ATTR_INDEX_TANGENT]   = sizeof(*tangents);

			vboSurf->vbo->sizes[ATTR_INDEX_POSITION]    = sizeof(*verts);
			vboSurf->vbo->sizes[ATTR_INDEX_NORMAL]      = sizeof(*normals);
			vboSurf->vbo->sizes[ATTR_INDEX_TEXCOORD0]   = sizeof(*texcoords);
			vboSurf->vbo->sizes[ATTR_INDEX_TANGENT]     = sizeof(*tangents);

			Z_Free(data);

			vboSurf->ibo = R_CreateIBO ((byte *)surf->indexes, sizeof (glIndex_t) * surf->numIndexes, VBO_USAGE_STATIC);
		}
	}

	return qtrue;
}



/*
=================
R_LoadMDR
=================
*/
static qboolean R_LoadMDR( model_t *mod, void *buffer, int filesize, const char *mod_name ) 
{
	int					i, j, k, l;
	mdrHeader_t			*pinmodel, *mdr;
	mdrFrame_t			*frame;
	mdrLOD_t			*lod, *curlod;
	mdrSurface_t			*surf, *cursurf;
	mdrTriangle_t			*tri, *curtri;
	mdrVertex_t			*v, *curv;
	mdrWeight_t			*weight, *curweight;
	mdrTag_t			*tag, *curtag;
	int					size;
	shader_t			*sh;

	pinmodel = (mdrHeader_t *)buffer;

	pinmodel->version = LittleLong(pinmodel->version);
	if (pinmodel->version != MDR_VERSION) 
	{
		ri->Printf(PRINT_WARNING, "R_LoadMDR: %s has wrong version (%i should be %i)\n", mod_name, pinmodel->version, MDR_VERSION);
		return qfalse;
	}

	size = LittleLong(pinmodel->ofsEnd);
	
	if(size > filesize)
	{
		ri->Printf(PRINT_WARNING, "R_LoadMDR: Header of %s is broken. Wrong filesize declared!\n", mod_name);
		return qfalse;
	}
	
	mod->type = MOD_MDR;

	LL(pinmodel->numFrames);
	LL(pinmodel->numBones);
	LL(pinmodel->ofsFrames);

	// This is a model that uses some type of compressed Bones. We don't want to uncompress every bone for each rendered frame
	// over and over again, we'll uncompress it in this function already, so we must adjust the size of the target mdr.
	if(pinmodel->ofsFrames < 0)
	{
		// mdrFrame_t is larger than mdrCompFrame_t:
		size += pinmodel->numFrames * sizeof(frame->name);
		// now add enough space for the uncompressed bones.
		size += pinmodel->numFrames * pinmodel->numBones * ((sizeof(mdrBone_t) - sizeof(mdrCompBone_t)));
	}
	
	// simple bounds check
	if(pinmodel->numBones < 0 ||
		sizeof(*mdr) + pinmodel->numFrames * (sizeof(*frame) + (pinmodel->numBones - 1) * sizeof(*frame->bones)) > size)
	{
		ri->Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
		return qfalse;
	}

	mod->dataSize += size;
	mod->data.mdr = mdr = (mdrHeader_t*)ri->Hunk_Alloc( size, h_low );

	// Copy all the values over from the file and fix endian issues in the process, if necessary.
	
	mdr->ident = LittleLong(pinmodel->ident);
	mdr->version = pinmodel->version;	// Don't need to swap byte order on this one, we already did above.
	Q_strncpyz(mdr->name, pinmodel->name, sizeof(mdr->name));
	mdr->numFrames = pinmodel->numFrames;
	mdr->numBones = pinmodel->numBones;
	mdr->numLODs = LittleLong(pinmodel->numLODs);
	mdr->numTags = LittleLong(pinmodel->numTags);
	// We don't care about the other offset values, we'll generate them ourselves while loading.

	mod->numLods = mdr->numLODs;

	if ( mdr->numFrames < 1 ) 
	{
		ri->Printf(PRINT_WARNING, "R_LoadMDR: %s has no frames\n", mod_name);
		return qfalse;
	}

	/* The first frame will be put into the first free space after the header */
	frame = (mdrFrame_t *)(mdr + 1);
	mdr->ofsFrames = (int)((byte *) frame - (byte *) mdr);
		
	if (pinmodel->ofsFrames < 0)
	{
		mdrCompFrame_t *cframe;
				
		// compressed model...				
		cframe = (mdrCompFrame_t *)((byte *) pinmodel - pinmodel->ofsFrames);
		
		for(i = 0; i < mdr->numFrames; i++)
		{
			for(j = 0; j < 3; j++)
			{
				frame->bounds[0][j] = LittleFloat(cframe->bounds[0][j]);
				frame->bounds[1][j] = LittleFloat(cframe->bounds[1][j]);
				frame->localOrigin[j] = LittleFloat(cframe->localOrigin[j]);
			}

			frame->radius = LittleFloat(cframe->radius);
			frame->name[0] = '\0';	// No name supplied in the compressed version.
			
			for(j = 0; j < mdr->numBones; j++)
			{
				for(k = 0; k < (sizeof(cframe->bones[j].Comp) / 2); k++)
				{
					// Do swapping for the uncompressing functions. They seem to use shorts
					// values only, so I assume this will work. Never tested it on other
					// platforms, though.
					
					((unsigned short *)(cframe->bones[j].Comp))[k] =
						LittleShort( ((unsigned short *)(cframe->bones[j].Comp))[k] );
				}
				
				/* Now do the actual uncompressing */
				MC_UnCompress(frame->bones[j].matrix, cframe->bones[j].Comp);
			}
			
			// Next Frame...
			cframe = (mdrCompFrame_t *) &cframe->bones[j];
			frame = (mdrFrame_t *) &frame->bones[j];
		}
	}
	else
	{
		mdrFrame_t *curframe;
		
		// uncompressed model...
		//
    
		curframe = (mdrFrame_t *)((byte *) pinmodel + pinmodel->ofsFrames);
		
		// swap all the frames
		for ( i = 0 ; i < mdr->numFrames ; i++) 
		{
			for(j = 0; j < 3; j++)
			{
				frame->bounds[0][j] = LittleFloat(curframe->bounds[0][j]);
				frame->bounds[1][j] = LittleFloat(curframe->bounds[1][j]);
				frame->localOrigin[j] = LittleFloat(curframe->localOrigin[j]);
			}
			
			frame->radius = LittleFloat(curframe->radius);
			Q_strncpyz(frame->name, curframe->name, sizeof(frame->name));
			
			for (j = 0; j < (int) (mdr->numBones * sizeof(mdrBone_t) / 4); j++) 
			{
				((float *)frame->bones)[j] = LittleFloat( ((float *)curframe->bones)[j] );
			}
			
			curframe = (mdrFrame_t *) &curframe->bones[mdr->numBones];
			frame = (mdrFrame_t *) &frame->bones[mdr->numBones];
		}
	}
	
	// frame should now point to the first free address after all frames.
	lod = (mdrLOD_t *) frame;
	mdr->ofsLODs = (int) ((byte *) lod - (byte *)mdr);
	
	curlod = (mdrLOD_t *)((byte *) pinmodel + LittleLong(pinmodel->ofsLODs));
		
	// swap all the LOD's
	for ( l = 0 ; l < mdr->numLODs ; l++)
	{
		// simple bounds check
		if((byte *) (lod + 1) > (byte *) mdr + size)
		{
			ri->Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
			return qfalse;
		}

		lod->numSurfaces = LittleLong(curlod->numSurfaces);
		
		// swap all the surfaces
		surf = (mdrSurface_t *) (lod + 1);
		lod->ofsSurfaces = (int)((byte *) surf - (byte *) lod);
		cursurf = (mdrSurface_t *) ((byte *)curlod + LittleLong(curlod->ofsSurfaces));
		
		for ( i = 0 ; i < lod->numSurfaces ; i++)
		{
			// simple bounds check
			if((byte *) (surf + 1) > (byte *) mdr + size)
			{
				ri->Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
				return qfalse;
			}

			// first do some copying stuff
			
			surf->ident = SF_MDR;
			Q_strncpyz(surf->name, cursurf->name, sizeof(surf->name));
			Q_strncpyz(surf->shader, cursurf->shader, sizeof(surf->shader));
			
			surf->ofsHeader = (byte *) mdr - (byte *) surf;
			
			surf->numVerts = LittleLong(cursurf->numVerts);
			surf->numTriangles = LittleLong(cursurf->numTriangles);
			// numBoneReferences and BoneReferences generally seem to be unused
			
			// now do the checks that may fail.
			if ( surf->numVerts >= SHADER_MAX_VERTEXES ) 
			{
				ri->Printf(PRINT_WARNING, "R_LoadMDR: %s has more than %i verts on %s (%i).\n",
					  mod_name, SHADER_MAX_VERTEXES-1, surf->name[0] ? surf->name : "a surface",
					  surf->numVerts );
				return qfalse;
			}
			if ( surf->numTriangles*3 >= SHADER_MAX_INDEXES ) 
			{
				ri->Printf(PRINT_WARNING, "R_LoadMDR: %s has more than %i triangles on %s (%i).\n",
					  mod_name, ( SHADER_MAX_INDEXES / 3 ) - 1, surf->name[0] ? surf->name : "a surface",
					  surf->numTriangles );
				return qfalse;
			}
			// lowercase the surface name so skin compares are faster
			Q_strlwr( surf->name );

			// register the shaders
			sh = R_FindShader(surf->shader, lightmapsNone, stylesDefault, qtrue);
			if ( sh->defaultShader ) {
				surf->shaderIndex = 0;
			} else {
				surf->shaderIndex = sh->index;
			}
			
			// now copy the vertexes.
			v = (mdrVertex_t *) (surf + 1);
			surf->ofsVerts = (int)((byte *) v - (byte *) surf);
			curv = (mdrVertex_t *) ((byte *)cursurf + LittleLong(cursurf->ofsVerts));
			
			for(j = 0; j < surf->numVerts; j++)
			{
				LL(curv->numWeights);
			
				// simple bounds check
				if(curv->numWeights < 0 || (byte *) (v + 1) + (curv->numWeights - 1) * sizeof(*weight) > (byte *) mdr + size)
				{
					ri->Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
					return qfalse;
				}

				v->normal[0] = LittleFloat(curv->normal[0]);
				v->normal[1] = LittleFloat(curv->normal[1]);
				v->normal[2] = LittleFloat(curv->normal[2]);
				
				v->texCoords[0] = LittleFloat(curv->texCoords[0]);
				v->texCoords[1] = LittleFloat(curv->texCoords[1]);
				
				v->numWeights = curv->numWeights;
				weight = &v->weights[0];
				curweight = &curv->weights[0];
				
				// Now copy all the weights
				for(k = 0; k < v->numWeights; k++)
				{
					weight->boneIndex = LittleLong(curweight->boneIndex);
					weight->boneWeight = LittleFloat(curweight->boneWeight);
					
					weight->offset[0] = LittleFloat(curweight->offset[0]);
					weight->offset[1] = LittleFloat(curweight->offset[1]);
					weight->offset[2] = LittleFloat(curweight->offset[2]);
					
					weight++;
					curweight++;
				}
				
				v = (mdrVertex_t *) weight;
				curv = (mdrVertex_t *) curweight;
			}
						
			// we know the offset to the triangles now:
			tri = (mdrTriangle_t *) v;
			surf->ofsTriangles = (int)((byte *) tri - (byte *) surf);
			curtri = (mdrTriangle_t *)((byte *) cursurf + LittleLong(cursurf->ofsTriangles));
			
			// simple bounds check
			if(surf->numTriangles < 0 || (byte *) (tri + surf->numTriangles) > (byte *) mdr + size)
			{
				ri->Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
				return qfalse;
			}

			for(j = 0; j < surf->numTriangles; j++)
			{
				tri->indexes[0] = LittleLong(curtri->indexes[0]);
				tri->indexes[1] = LittleLong(curtri->indexes[1]);
				tri->indexes[2] = LittleLong(curtri->indexes[2]);
				
				tri++;
				curtri++;
			}
			
			// tri now points to the end of the surface.
			surf->ofsEnd = (byte *) tri - (byte *) surf;
			surf = (mdrSurface_t *) tri;

			// find the next surface.
			cursurf = (mdrSurface_t *) ((byte *) cursurf + LittleLong(cursurf->ofsEnd));
		}

		// surf points to the next lod now.
		lod->ofsEnd = (int)((byte *) surf - (byte *) lod);
		lod = (mdrLOD_t *) surf;

		// find the next LOD.
		curlod = (mdrLOD_t *)((byte *) curlod + LittleLong(curlod->ofsEnd));
	}
	
	// lod points to the first tag now, so update the offset too.
	tag = (mdrTag_t *) lod;
	mdr->ofsTags = (int)((byte *) tag - (byte *) mdr);
	curtag = (mdrTag_t *) ((byte *)pinmodel + LittleLong(pinmodel->ofsTags));

	// simple bounds check
	if(mdr->numTags < 0 || (byte *) (tag + mdr->numTags) > (byte *) mdr + size)
	{
		ri->Printf(PRINT_WARNING, "R_LoadMDR: %s has broken structure.\n", mod_name);
		return qfalse;
	}
	
	for (i = 0 ; i < mdr->numTags ; i++)
	{
		tag->boneIndex = LittleLong(curtag->boneIndex);
		Q_strncpyz(tag->name, curtag->name, sizeof(tag->name));
		
		tag++;
		curtag++;
	}
	
	// And finally we know the real offset to the end.
	mdr->ofsEnd = (int)((byte *) tag - (byte *) mdr);

	// phew! we're done.
	
	return qtrue;
}



//=============================================================================

static void R_InitBuiltInModels()
{
#if 0
	const int NUM_SIDES = 18;

	vec3_t positions[NUM_SIDES*(NUM_SIDES-1) + 2]; // +2 for vertices at poles
	int numPositions = 0;
	positions[0][0] = 0.0f;
	positions[0][1] = 1.0f;
	positions[0][2] = 0.0f;
	numPositions++;

	positions[1][0] = 0.0f;
	positions[1][1] = -1.0f;
	positions[1][2] = 0.0f;
	numPositions++;

	for ( int y = 1; y < NUM_SIDES; y++ )
	{
		float angleFromUpVector = y * M_PI / (float)NUM_SIDES;
		float ypos = cosf(angleFromUpVector);
		float ringRadius = sinf(angleFromUpVector);

		for ( int x = 0; x < NUM_SIDES; x++ )
		{
			float angleFromForwardVector = x * 2.0f * M_PI / (float)NUM_SIDES;
			float xpos = ringRadius * sinf(angleFromForwardVector);
			float zpos = ringRadius * cosf(angleFromForwardVector);

			positions[numPositions][0] = xpos;
			positions[numPositions][1] = ypos;
			positions[numPositions][2] = zpos;
			numPositions++;
		}
	}

	assert(numPositions == ARRAY_LEN(positions));

	glIndex_t indices[2048];
	glIndex_t *tri = indices;

	// Indices at poles
	int startIndexOfFirstRow = 2;
	int startIndexOfLastRow = startIndexOfFirstRow + NUM_SIDES * (NUM_SIDES - 2);

	for ( int i = 0; i < NUM_SIDES; i++ )
	{
		tri[0] = 0;
		tri[1] = startIndexOfFirstRow + ((i + 1) % NUM_SIDES);
		tri[2] = startIndexOfFirstRow + i;
		tri += 3;

		tri[0] = 1;
		tri[1] = startIndexOfLastRow + i;
		tri[2] = startIndexOfLastRow + ((i + 1) % NUM_SIDES);
		tri += 3;
	}

	int startOfRow = startIndexOfFirstRow;
	int startOfNextRow = startOfRow + NUM_SIDES;
	int biggestIndex = 0;
	for ( int y = 1; y < (NUM_SIDES - 1); y++ )
	{
		for ( int x = 0; x < NUM_SIDES; x++ )
		{
			int i0 = startOfRow + x;
			int i1 = startOfRow + ((x + 1) % NUM_SIDES);
			int i2 = startOfNextRow + x;
			int i3 = startOfNextRow + ((x + 1) % NUM_SIDES);

			assert(i0 < numPositions);
			assert(i1 < numPositions);
			assert(i2 < numPositions);
			assert(i3 < numPositions);

			tri[0] = i0;
			tri[1] = i3;
			tri[2] = i2;
			tri += 3;

			tri[0] = i0;
			tri[1] = i1;
			tri[2] = i3;
			tri += 3;

			biggestIndex = Q_max(i0, biggestIndex);
			biggestIndex = Q_max(i1, biggestIndex);
			biggestIndex = Q_max(i2, biggestIndex);
			biggestIndex = Q_max(i3, biggestIndex);
		}

		startOfRow = startOfNextRow;
		startOfNextRow += NUM_SIDES;
	}

	assert(biggestIndex == numPositions-1);
	int numIndices = tri - indices;

	assert(numIndices < ARRAY_LEN(indices));

	Com_Printf("vec3_t positions[] = {");
	for ( int i = 0; i < numPositions; i++ )
	{
		Com_Printf("{%.6ff, %.6ff, %.6ff}, ", positions[i][0], positions[i][1], positions[i][2]);
		if ( !(i % 3) )
			Com_Printf("\n");
	}
	Com_Printf("};\n");

	Com_Printf("glIndex_t indices[] = {");
	for ( int i = 0 ; i < numIndices; i++ )
	{
		Com_Printf("%d, ", indices[i]);
		if ( !(i % 25) )
			Com_Printf("\n");
	}
	Com_Printf("};\n");
#else
	static const vec3_t positions[] = {
		{0.000000f, 1.000000f, 0.000000f}, {0.000000f, -1.000000f, 0.000000f}, {0.000000f, 0.984808f, 0.173648f},
		{0.059391f, 0.984808f, 0.163176f}, {0.111619f, 0.984808f, 0.133022f}, {0.150384f, 0.984808f, 0.086824f},
		{0.171010f, 0.984808f, 0.030154f}, {0.171010f, 0.984808f, -0.030154f}, {0.150384f, 0.984808f, -0.086824f},
		{0.111619f, 0.984808f, -0.133022f}, {0.059391f, 0.984808f, -0.163176f}, {-0.000000f, 0.984808f, -0.173648f},
		{-0.059391f, 0.984808f, -0.163176f}, {-0.111619f, 0.984808f, -0.133022f}, {-0.150384f, 0.984808f, -0.086824f},
		{-0.171010f, 0.984808f, -0.030154f}, {-0.171010f, 0.984808f, 0.030154f}, {-0.150384f, 0.984808f, 0.086824f},
		{-0.111619f, 0.984808f, 0.133022f}, {-0.059391f, 0.984808f, 0.163176f}, {0.000000f, 0.939693f, 0.342020f},
		{0.116978f, 0.939693f, 0.321394f}, {0.219846f, 0.939693f, 0.262003f}, {0.296198f, 0.939693f, 0.171010f},
		{0.336824f, 0.939693f, 0.059391f}, {0.336824f, 0.939693f, -0.059391f}, {0.296198f, 0.939693f, -0.171010f},
		{0.219846f, 0.939693f, -0.262003f}, {0.116978f, 0.939693f, -0.321394f}, {-0.000000f, 0.939693f, -0.342020f},
		{-0.116978f, 0.939693f, -0.321394f}, {-0.219846f, 0.939693f, -0.262003f}, {-0.296198f, 0.939693f, -0.171010f},
		{-0.336824f, 0.939693f, -0.059391f}, {-0.336824f, 0.939693f, 0.059391f}, {-0.296198f, 0.939693f, 0.171010f},
		{-0.219846f, 0.939693f, 0.262003f}, {-0.116978f, 0.939693f, 0.321394f}, {0.000000f, 0.866025f, 0.500000f},
		{0.171010f, 0.866025f, 0.469846f}, {0.321394f, 0.866025f, 0.383022f}, {0.433013f, 0.866025f, 0.250000f},
		{0.492404f, 0.866025f, 0.086824f}, {0.492404f, 0.866025f, -0.086824f}, {0.433013f, 0.866025f, -0.250000f},
		{0.321394f, 0.866025f, -0.383022f}, {0.171010f, 0.866025f, -0.469846f}, {-0.000000f, 0.866025f, -0.500000f},
		{-0.171010f, 0.866025f, -0.469846f}, {-0.321394f, 0.866025f, -0.383022f}, {-0.433013f, 0.866025f, -0.250000f},
		{-0.492404f, 0.866025f, -0.086824f}, {-0.492404f, 0.866025f, 0.086824f}, {-0.433013f, 0.866025f, 0.250000f},
		{-0.321394f, 0.866025f, 0.383022f}, {-0.171010f, 0.866025f, 0.469846f}, {0.000000f, 0.766044f, 0.642788f},
		{0.219846f, 0.766044f, 0.604023f}, {0.413176f, 0.766044f, 0.492404f}, {0.556670f, 0.766044f, 0.321394f},
		{0.633022f, 0.766044f, 0.111619f}, {0.633022f, 0.766044f, -0.111619f}, {0.556670f, 0.766044f, -0.321394f},
		{0.413176f, 0.766044f, -0.492404f}, {0.219846f, 0.766044f, -0.604023f}, {-0.000000f, 0.766044f, -0.642788f},
		{-0.219846f, 0.766044f, -0.604023f}, {-0.413176f, 0.766044f, -0.492404f}, {-0.556670f, 0.766044f, -0.321394f},
		{-0.633022f, 0.766044f, -0.111619f}, {-0.633022f, 0.766044f, 0.111619f}, {-0.556670f, 0.766044f, 0.321394f},
		{-0.413176f, 0.766044f, 0.492404f}, {-0.219846f, 0.766044f, 0.604023f}, {0.000000f, 0.642788f, 0.766044f},
		{0.262003f, 0.642788f, 0.719846f}, {0.492404f, 0.642788f, 0.586824f}, {0.663414f, 0.642788f, 0.383022f},
		{0.754407f, 0.642788f, 0.133022f}, {0.754407f, 0.642788f, -0.133022f}, {0.663414f, 0.642788f, -0.383022f},
		{0.492404f, 0.642788f, -0.586824f}, {0.262003f, 0.642788f, -0.719846f}, {-0.000000f, 0.642788f, -0.766044f},
		{-0.262003f, 0.642788f, -0.719846f}, {-0.492404f, 0.642788f, -0.586824f}, {-0.663414f, 0.642788f, -0.383022f},
		{-0.754407f, 0.642788f, -0.133022f}, {-0.754407f, 0.642788f, 0.133023f}, {-0.663414f, 0.642788f, 0.383022f},
		{-0.492404f, 0.642788f, 0.586824f}, {-0.262002f, 0.642788f, 0.719846f}, {0.000000f, 0.500000f, 0.866025f},
		{0.296198f, 0.500000f, 0.813798f}, {0.556670f, 0.500000f, 0.663414f}, {0.750000f, 0.500000f, 0.433013f},
		{0.852869f, 0.500000f, 0.150384f}, {0.852869f, 0.500000f, -0.150384f}, {0.750000f, 0.500000f, -0.433013f},
		{0.556670f, 0.500000f, -0.663414f}, {0.296198f, 0.500000f, -0.813798f}, {-0.000000f, 0.500000f, -0.866025f},
		{-0.296198f, 0.500000f, -0.813798f}, {-0.556671f, 0.500000f, -0.663414f}, {-0.750000f, 0.500000f, -0.433013f},
		{-0.852869f, 0.500000f, -0.150384f}, {-0.852868f, 0.500000f, 0.150384f}, {-0.750000f, 0.500000f, 0.433013f},
		{-0.556670f, 0.500000f, 0.663414f}, {-0.296198f, 0.500000f, 0.813798f}, {0.000000f, 0.342020f, 0.939693f},
		{0.321394f, 0.342020f, 0.883022f}, {0.604023f, 0.342020f, 0.719846f}, {0.813798f, 0.342020f, 0.469846f},
		{0.925417f, 0.342020f, 0.163176f}, {0.925417f, 0.342020f, -0.163176f}, {0.813798f, 0.342020f, -0.469846f},
		{0.604023f, 0.342020f, -0.719846f}, {0.321394f, 0.342020f, -0.883022f}, {-0.000000f, 0.342020f, -0.939693f},
		{-0.321394f, 0.342020f, -0.883022f}, {-0.604023f, 0.342020f, -0.719846f}, {-0.813798f, 0.342020f, -0.469846f},
		{-0.925417f, 0.342020f, -0.163176f}, {-0.925417f, 0.342020f, 0.163176f}, {-0.813798f, 0.342020f, 0.469846f},
		{-0.604023f, 0.342020f, 0.719846f}, {-0.321394f, 0.342020f, 0.883022f}, {0.000000f, 0.173648f, 0.984808f},
		{0.336824f, 0.173648f, 0.925417f}, {0.633022f, 0.173648f, 0.754407f}, {0.852869f, 0.173648f, 0.492404f},
		{0.969846f, 0.173648f, 0.171010f}, {0.969846f, 0.173648f, -0.171010f}, {0.852869f, 0.173648f, -0.492404f},
		{0.633022f, 0.173648f, -0.754407f}, {0.336824f, 0.173648f, -0.925417f}, {-0.000000f, 0.173648f, -0.984808f},
		{-0.336824f, 0.173648f, -0.925417f}, {-0.633022f, 0.173648f, -0.754406f}, {-0.852869f, 0.173648f, -0.492404f},
		{-0.969846f, 0.173648f, -0.171010f}, {-0.969846f, 0.173648f, 0.171011f}, {-0.852869f, 0.173648f, 0.492404f},
		{-0.633022f, 0.173648f, 0.754407f}, {-0.336824f, 0.173648f, 0.925417f}, {0.000000f, -0.000000f, 1.000000f},
		{0.342020f, -0.000000f, 0.939693f}, {0.642788f, -0.000000f, 0.766044f}, {0.866025f, -0.000000f, 0.500000f},
		{0.984808f, -0.000000f, 0.173648f}, {0.984808f, -0.000000f, -0.173648f}, {0.866025f, -0.000000f, -0.500000f},
		{0.642787f, -0.000000f, -0.766045f}, {0.342020f, -0.000000f, -0.939693f}, {-0.000000f, -0.000000f, -1.000000f},
		{-0.342020f, -0.000000f, -0.939693f}, {-0.642788f, -0.000000f, -0.766044f}, {-0.866025f, -0.000000f, -0.500000f},
		{-0.984808f, -0.000000f, -0.173648f}, {-0.984808f, -0.000000f, 0.173649f}, {-0.866025f, -0.000000f, 0.500000f},
		{-0.642787f, -0.000000f, 0.766045f}, {-0.342020f, -0.000000f, 0.939693f}, {0.000000f, -0.173648f, 0.984808f},
		{0.336824f, -0.173648f, 0.925417f}, {0.633022f, -0.173648f, 0.754407f}, {0.852869f, -0.173648f, 0.492404f},
		{0.969846f, -0.173648f, 0.171010f}, {0.969846f, -0.173648f, -0.171010f}, {0.852868f, -0.173648f, -0.492404f},
		{0.633022f, -0.173648f, -0.754407f}, {0.336824f, -0.173648f, -0.925417f}, {-0.000000f, -0.173648f, -0.984808f},
		{-0.336824f, -0.173648f, -0.925416f}, {-0.633022f, -0.173648f, -0.754406f}, {-0.852869f, -0.173648f, -0.492404f},
		{-0.969846f, -0.173648f, -0.171010f}, {-0.969846f, -0.173648f, 0.171011f}, {-0.852869f, -0.173648f, 0.492404f},
		{-0.633022f, -0.173648f, 0.754407f}, {-0.336824f, -0.173648f, 0.925417f}, {0.000000f, -0.342020f, 0.939693f},
		{0.321394f, -0.342020f, 0.883022f}, {0.604023f, -0.342020f, 0.719846f}, {0.813798f, -0.342020f, 0.469846f},
		{0.925417f, -0.342020f, 0.163176f}, {0.925417f, -0.342020f, -0.163176f}, {0.813798f, -0.342020f, -0.469846f},
		{0.604023f, -0.342020f, -0.719846f}, {0.321394f, -0.342020f, -0.883022f}, {-0.000000f, -0.342020f, -0.939693f},
		{-0.321394f, -0.342020f, -0.883022f}, {-0.604023f, -0.342020f, -0.719846f}, {-0.813798f, -0.342020f, -0.469846f},
		{-0.925417f, -0.342020f, -0.163176f}, {-0.925416f, -0.342020f, 0.163176f}, {-0.813798f, -0.342020f, 0.469846f},
		{-0.604023f, -0.342020f, 0.719846f}, {-0.321394f, -0.342020f, 0.883022f}, {0.000000f, -0.500000f, 0.866025f},
		{0.296198f, -0.500000f, 0.813798f}, {0.556670f, -0.500000f, 0.663414f}, {0.750000f, -0.500000f, 0.433013f},
		{0.852869f, -0.500000f, 0.150384f}, {0.852868f, -0.500000f, -0.150384f}, {0.750000f, -0.500000f, -0.433013f},
		{0.556670f, -0.500000f, -0.663414f}, {0.296198f, -0.500000f, -0.813798f}, {-0.000000f, -0.500000f, -0.866025f},
		{-0.296198f, -0.500000f, -0.813798f}, {-0.556670f, -0.500000f, -0.663414f}, {-0.750000f, -0.500000f, -0.433013f},
		{-0.852869f, -0.500000f, -0.150384f}, {-0.852868f, -0.500000f, 0.150384f}, {-0.750000f, -0.500000f, 0.433013f},
		{-0.556670f, -0.500000f, 0.663414f}, {-0.296198f, -0.500000f, 0.813798f}, {0.000000f, -0.642788f, 0.766044f},
		{0.262003f, -0.642788f, 0.719846f}, {0.492404f, -0.642788f, 0.586824f}, {0.663414f, -0.642788f, 0.383022f},
		{0.754407f, -0.642788f, 0.133022f}, {0.754407f, -0.642788f, -0.133022f}, {0.663414f, -0.642788f, -0.383022f},
		{0.492404f, -0.642788f, -0.586824f}, {0.262003f, -0.642788f, -0.719846f}, {-0.000000f, -0.642788f, -0.766044f},
		{-0.262003f, -0.642788f, -0.719846f}, {-0.492404f, -0.642788f, -0.586824f}, {-0.663414f, -0.642788f, -0.383022f},
		{-0.754407f, -0.642788f, -0.133022f}, {-0.754406f, -0.642788f, 0.133023f}, {-0.663414f, -0.642788f, 0.383022f},
		{-0.492404f, -0.642788f, 0.586824f}, {-0.262002f, -0.642788f, 0.719846f}, {0.000000f, -0.766045f, 0.642787f},
		{0.219846f, -0.766045f, 0.604023f}, {0.413176f, -0.766045f, 0.492404f}, {0.556670f, -0.766045f, 0.321394f},
		{0.633022f, -0.766045f, 0.111619f}, {0.633022f, -0.766045f, -0.111619f}, {0.556670f, -0.766045f, -0.321394f},
		{0.413176f, -0.766045f, -0.492404f}, {0.219846f, -0.766045f, -0.604023f}, {-0.000000f, -0.766045f, -0.642787f},
		{-0.219846f, -0.766045f, -0.604023f}, {-0.413176f, -0.766045f, -0.492404f}, {-0.556670f, -0.766045f, -0.321394f},
		{-0.633022f, -0.766045f, -0.111619f}, {-0.633022f, -0.766045f, 0.111619f}, {-0.556670f, -0.766045f, 0.321394f},
		{-0.413176f, -0.766045f, 0.492404f}, {-0.219846f, -0.766045f, 0.604023f}, {0.000000f, -0.866025f, 0.500000f},
		{0.171010f, -0.866025f, 0.469846f}, {0.321394f, -0.866025f, 0.383022f}, {0.433013f, -0.866025f, 0.250000f},
		{0.492404f, -0.866025f, 0.086824f}, {0.492404f, -0.866025f, -0.086824f}, {0.433013f, -0.866025f, -0.250000f},
		{0.321394f, -0.866025f, -0.383022f}, {0.171010f, -0.866025f, -0.469846f}, {-0.000000f, -0.866025f, -0.500000f},
		{-0.171010f, -0.866025f, -0.469846f}, {-0.321394f, -0.866025f, -0.383022f}, {-0.433013f, -0.866025f, -0.250000f},
		{-0.492404f, -0.866025f, -0.086824f}, {-0.492404f, -0.866025f, 0.086824f}, {-0.433013f, -0.866025f, 0.250000f},
		{-0.321394f, -0.866025f, 0.383022f}, {-0.171010f, -0.866025f, 0.469846f}, {0.000000f, -0.939693f, 0.342020f},
		{0.116978f, -0.939693f, 0.321394f}, {0.219846f, -0.939693f, 0.262003f}, {0.296198f, -0.939693f, 0.171010f},
		{0.336824f, -0.939693f, 0.059391f}, {0.336824f, -0.939693f, -0.059391f}, {0.296198f, -0.939693f, -0.171010f},
		{0.219846f, -0.939693f, -0.262003f}, {0.116978f, -0.939693f, -0.321394f}, {-0.000000f, -0.939693f, -0.342020f},
		{-0.116978f, -0.939693f, -0.321394f}, {-0.219846f, -0.939693f, -0.262002f}, {-0.296198f, -0.939693f, -0.171010f},
		{-0.336824f, -0.939693f, -0.059391f}, {-0.336824f, -0.939693f, 0.059391f}, {-0.296198f, -0.939693f, 0.171010f},
		{-0.219846f, -0.939693f, 0.262003f}, {-0.116978f, -0.939693f, 0.321394f}, {0.000000f, -0.984808f, 0.173648f},
		{0.059391f, -0.984808f, 0.163176f}, {0.111619f, -0.984808f, 0.133022f}, {0.150384f, -0.984808f, 0.086824f},
		{0.171010f, -0.984808f, 0.030154f}, {0.171010f, -0.984808f, -0.030154f}, {0.150384f, -0.984808f, -0.086824f},
		{0.111619f, -0.984808f, -0.133022f}, {0.059391f, -0.984808f, -0.163176f}, {-0.000000f, -0.984808f, -0.173648f},
		{-0.059391f, -0.984808f, -0.163176f}, {-0.111619f, -0.984808f, -0.133022f}, {-0.150384f, -0.984808f, -0.086824f},
		{-0.171010f, -0.984808f, -0.030154f}, {-0.171010f, -0.984808f, 0.030154f}, {-0.150384f, -0.984808f, 0.086824f},
		{-0.111619f, -0.984808f, 0.133022f}, {-0.059391f, -0.984808f, 0.163176f}
	};

	static const glIndex_t indices[] = {
		0, 3, 2, 1, 290, 291, 0, 4, 3, 1, 291, 292, 0, 5, 4, 1, 292, 293, 0,
		6, 5, 1, 293, 294, 0, 7, 6, 1, 294, 295, 0, 8, 7, 1, 295, 296, 0, 9,
		8, 1, 296, 297, 0, 10, 9, 1, 297, 298, 0, 11, 10, 1, 298, 299, 0, 12,
		11, 1, 299, 300, 0, 13, 12, 1, 300, 301, 0, 14, 13, 1, 301, 302, 0, 15,
		14, 1, 302, 303, 0, 16, 15, 1, 303, 304, 0, 17, 16, 1, 304, 305, 0, 18,
		17, 1, 305, 306, 0, 19, 18, 1, 306, 307, 0, 2, 19, 1, 307, 290, 2, 21,
		20, 2, 3, 21, 3, 22, 21, 3, 4, 22, 4, 23, 22, 4, 5, 23, 5, 24, 23, 5,
		6, 24, 6, 25, 24, 6, 7, 25, 7, 26, 25, 7, 8, 26, 8, 27, 26, 8, 9, 27,
		9, 28, 27, 9, 10, 28, 10, 29, 28, 10, 11, 29, 11, 30, 29, 11, 12, 30,
		12, 31, 30, 12, 13, 31, 13, 32, 31, 13, 14, 32, 14, 33, 32, 14, 15, 33,
		15, 34, 33, 15, 16, 34, 16, 35, 34, 16, 17, 35, 17, 36, 35, 17, 18, 36,
		18, 37, 36, 18, 19, 37, 19, 20, 37, 19, 2, 20, 20, 39, 38, 20, 21, 39,
		21, 40, 39, 21, 22, 40, 22, 41, 40, 22, 23, 41, 23, 42, 41, 23, 24, 42,
		24, 43, 42, 24, 25, 43, 25, 44, 43, 25, 26, 44, 26, 45, 44, 26, 27, 45,
		27, 46, 45, 27, 28, 46, 28, 47, 46, 28, 29, 47, 29, 48, 47, 29, 30, 48,
		30, 49, 48, 30, 31, 49, 31, 50, 49, 31, 32, 50, 32, 51, 50, 32, 33, 51,
		33, 52, 51, 33, 34, 52, 34, 53, 52, 34, 35, 53, 35, 54, 53, 35, 36, 54,
		36, 55, 54, 36, 37, 55, 37, 38, 55, 37, 20, 38, 38, 57, 56, 38, 39, 57,
		39, 58, 57, 39, 40, 58, 40, 59, 58, 40, 41, 59, 41, 60, 59, 41, 42, 60,
		42, 61, 60, 42, 43, 61, 43, 62, 61, 43, 44, 62, 44, 63, 62, 44, 45, 63,
		45, 64, 63, 45, 46, 64, 46, 65, 64, 46, 47, 65, 47, 66, 65, 47, 48, 66,
		48, 67, 66, 48, 49, 67, 49, 68, 67, 49, 50, 68, 50, 69, 68, 50, 51, 69,
		51, 70, 69, 51, 52, 70, 52, 71, 70, 52, 53, 71, 53, 72, 71, 53, 54, 72,
		54, 73, 72, 54, 55, 73, 55, 56, 73, 55, 38, 56, 56, 75, 74, 56, 57, 75,
		57, 76, 75, 57, 58, 76, 58, 77, 76, 58, 59, 77, 59, 78, 77, 59, 60, 78,
		60, 79, 78, 60, 61, 79, 61, 80, 79, 61, 62, 80, 62, 81, 80, 62, 63, 81,
		63, 82, 81, 63, 64, 82, 64, 83, 82, 64, 65, 83, 65, 84, 83, 65, 66, 84,
		66, 85, 84, 66, 67, 85, 67, 86, 85, 67, 68, 86, 68, 87, 86, 68, 69, 87,
		69, 88, 87, 69, 70, 88, 70, 89, 88, 70, 71, 89, 71, 90, 89, 71, 72, 90,
		72, 91, 90, 72, 73, 91, 73, 74, 91, 73, 56, 74, 74, 93, 92, 74, 75, 93,
		75, 94, 93, 75, 76, 94, 76, 95, 94, 76, 77, 95, 77, 96, 95, 77, 78, 96,
		78, 97, 96, 78, 79, 97, 79, 98, 97, 79, 80, 98, 80, 99, 98, 80, 81, 99,
		81, 100, 99, 81, 82, 100, 82, 101, 100, 82, 83, 101, 83, 102, 101, 83,
		84, 102, 84, 103, 102, 84, 85, 103, 85, 104, 103, 85, 86, 104, 86, 105,
		104, 86, 87, 105, 87, 106, 105, 87, 88, 106, 88, 107, 106, 88, 89, 107,
		89, 108, 107, 89, 90, 108, 90, 109, 108, 90, 91, 109, 91, 92, 109, 91,
		74, 92, 92, 111, 110, 92, 93, 111, 93, 112, 111, 93, 94, 112, 94, 113,
		112, 94, 95, 113, 95, 114, 113, 95, 96, 114, 96, 115, 114, 96, 97, 115,
		97, 116, 115, 97, 98, 116, 98, 117, 116, 98, 99, 117, 99, 118, 117, 99,
		100, 118, 100, 119, 118, 100, 101, 119, 101, 120, 119, 101, 102, 120,
		102, 121, 120, 102, 103, 121, 103, 122, 121, 103, 104, 122, 104, 123,
		122, 104, 105, 123, 105, 124, 123, 105, 106, 124, 106, 125, 124, 106,
		107, 125, 107, 126, 125, 107, 108, 126, 108, 127, 126, 108, 109, 127,
		109, 110, 127, 109, 92, 110, 110, 129, 128, 110, 111, 129, 111, 130,
		129, 111, 112, 130, 112, 131, 130, 112, 113, 131, 113, 132, 131, 113,
		114, 132, 114, 133, 132, 114, 115, 133, 115, 134, 133, 115, 116, 134,
		116, 135, 134, 116, 117, 135, 117, 136, 135, 117, 118, 136, 118, 137,
		136, 118, 119, 137, 119, 138, 137, 119, 120, 138, 120, 139, 138, 120,
		121, 139, 121, 140, 139, 121, 122, 140, 122, 141, 140, 122, 123, 141,
		123, 142, 141, 123, 124, 142, 124, 143, 142, 124, 125, 143, 125, 144,
		143, 125, 126, 144, 126, 145, 144, 126, 127, 145, 127, 128, 145, 127,
		110, 128, 128, 147, 146, 128, 129, 147, 129, 148, 147, 129, 130, 148,
		130, 149, 148, 130, 131, 149, 131, 150, 149, 131, 132, 150, 132, 151,
		150, 132, 133, 151, 133, 152, 151, 133, 134, 152, 134, 153, 152, 134,
		135, 153, 135, 154, 153, 135, 136, 154, 136, 155, 154, 136, 137, 155,
		137, 156, 155, 137, 138, 156, 138, 157, 156, 138, 139, 157, 139, 158,
		157, 139, 140, 158, 140, 159, 158, 140, 141, 159, 141, 160, 159, 141,
		142, 160, 142, 161, 160, 142, 143, 161, 143, 162, 161, 143, 144, 162,
		144, 163, 162, 144, 145, 163, 145, 146, 163, 145, 128, 146, 146, 165,
		164, 146, 147, 165, 147, 166, 165, 147, 148, 166, 148, 167, 166, 148,
		149, 167, 149, 168, 167, 149, 150, 168, 150, 169, 168, 150, 151, 169,
		151, 170, 169, 151, 152, 170, 152, 171, 170, 152, 153, 171, 153, 172,
		171, 153, 154, 172, 154, 173, 172, 154, 155, 173, 155, 174, 173, 155,
		156, 174, 156, 175, 174, 156, 157, 175, 157, 176, 175, 157, 158, 176,
		158, 177, 176, 158, 159, 177, 159, 178, 177, 159, 160, 178, 160, 179,
		178, 160, 161, 179, 161, 180, 179, 161, 162, 180, 162, 181, 180, 162,
		163, 181, 163, 164, 181, 163, 146, 164, 164, 183, 182, 164, 165, 183,
		165, 184, 183, 165, 166, 184, 166, 185, 184, 166, 167, 185, 167, 186,
		185, 167, 168, 186, 168, 187, 186, 168, 169, 187, 169, 188, 187, 169,
		170, 188, 170, 189, 188, 170, 171, 189, 171, 190, 189, 171, 172, 190,
		172, 191, 190, 172, 173, 191, 173, 192, 191, 173, 174, 192, 174, 193,
		192, 174, 175, 193, 175, 194, 193, 175, 176, 194, 176, 195, 194, 176,
		177, 195, 177, 196, 195, 177, 178, 196, 178, 197, 196, 178, 179, 197,
		179, 198, 197, 179, 180, 198, 180, 199, 198, 180, 181, 199, 181, 182,
		199, 181, 164, 182, 182, 201, 200, 182, 183, 201, 183, 202, 201, 183,
		184, 202, 184, 203, 202, 184, 185, 203, 185, 204, 203, 185, 186, 204,
		186, 205, 204, 186, 187, 205, 187, 206, 205, 187, 188, 206, 188, 207,
		206, 188, 189, 207, 189, 208, 207, 189, 190, 208, 190, 209, 208, 190,
		191, 209, 191, 210, 209, 191, 192, 210, 192, 211, 210, 192, 193, 211,
		193, 212, 211, 193, 194, 212, 194, 213, 212, 194, 195, 213, 195, 214,
		213, 195, 196, 214, 196, 215, 214, 196, 197, 215, 197, 216, 215, 197,
		198, 216, 198, 217, 216, 198, 199, 217, 199, 200, 217, 199, 182, 200,
		200, 219, 218, 200, 201, 219, 201, 220, 219, 201, 202, 220, 202, 221,
		220, 202, 203, 221, 203, 222, 221, 203, 204, 222, 204, 223, 222, 204,
		205, 223, 205, 224, 223, 205, 206, 224, 206, 225, 224, 206, 207, 225,
		207, 226, 225, 207, 208, 226, 208, 227, 226, 208, 209, 227, 209, 228,
		227, 209, 210, 228, 210, 229, 228, 210, 211, 229, 211, 230, 229, 211,
		212, 230, 212, 231, 230, 212, 213, 231, 213, 232, 231, 213, 214, 232,
		214, 233, 232, 214, 215, 233, 215, 234, 233, 215, 216, 234, 216, 235,
		234, 216, 217, 235, 217, 218, 235, 217, 200, 218, 218, 237, 236, 218,
		219, 237, 219, 238, 237, 219, 220, 238, 220, 239, 238, 220, 221, 239,
		221, 240, 239, 221, 222, 240, 222, 241, 240, 222, 223, 241, 223, 242,
		241, 223, 224, 242, 224, 243, 242, 224, 225, 243, 225, 244, 243, 225,
		226, 244, 226, 245, 244, 226, 227, 245, 227, 246, 245, 227, 228, 246,
		228, 247, 246, 228, 229, 247, 229, 248, 247, 229, 230, 248, 230, 249,
		248, 230, 231, 249, 231, 250, 249, 231, 232, 250, 232, 251, 250, 232,
		233, 251, 233, 252, 251, 233, 234, 252, 234, 253, 252, 234, 235, 253,
		235, 236, 253, 235, 218, 236, 236, 255, 254, 236, 237, 255, 237, 256,
		255, 237, 238, 256, 238, 257, 256, 238, 239, 257, 239, 258, 257, 239,
		240, 258, 240, 259, 258, 240, 241, 259, 241, 260, 259, 241, 242, 260,
		242, 261, 260, 242, 243, 261, 243, 262, 261, 243, 244, 262, 244, 263,
		262, 244, 245, 263, 245, 264, 263, 245, 246, 264, 246, 265, 264, 246,
		247, 265, 247, 266, 265, 247, 248, 266, 248, 267, 266, 248, 249, 267,
		249, 268, 267, 249, 250, 268, 250, 269, 268, 250, 251, 269, 251, 270,
		269, 251, 252, 270, 252, 271, 270, 252, 253, 271, 253, 254, 271, 253,
		236, 254, 254, 273, 272, 254, 255, 273, 255, 274, 273, 255, 256, 274,
		256, 275, 274, 256, 257, 275, 257, 276, 275, 257, 258, 276, 258, 277,
		276, 258, 259, 277, 259, 278, 277, 259, 260, 278, 260, 279, 278, 260,
		261, 279, 261, 280, 279, 261, 262, 280, 262, 281, 280, 262, 263, 281,
		263, 282, 281, 263, 264, 282, 264, 283, 282, 264, 265, 283, 265, 284,
		283, 265, 266, 284, 266, 285, 284, 266, 267, 285, 267, 286, 285, 267,
		268, 286, 268, 287, 286, 268, 269, 287, 269, 288, 287, 269, 270, 288,
		270, 289, 288, 270, 271, 289, 271, 272, 289, 271, 254, 272, 272, 291,
		290, 272, 273, 291, 273, 292, 291, 273, 274, 292, 274, 293, 292, 274,
		275, 293, 275, 294, 293, 275, 276, 294, 276, 295, 294, 276, 277, 295,
		277, 296, 295, 277, 278, 296, 278, 297, 296, 278, 279, 297, 279, 298,
		297, 279, 280, 298, 280, 299, 298, 280, 281, 299, 281, 300, 299, 281,
		282, 300, 282, 301, 300, 282, 283, 301, 283, 302, 301, 283, 284, 302,
		284, 303, 302, 284, 285, 303, 285, 304, 303, 285, 286, 304, 286, 305,
		304, 286, 287, 305, 287, 306, 305, 287, 288, 306, 288, 307, 306, 288,
		289, 307, 289, 290, 307, 289, 272, 290
	};
#endif

	tr.lightSphereVolume.ibo = R_CreateIBO((byte *)indices, sizeof(indices), VBO_USAGE_STATIC);
	tr.lightSphereVolume.vbo = R_CreateVBO((byte *)positions, sizeof(positions), VBO_USAGE_STATIC);
	tr.lightSphereVolume.numIndexes = ARRAY_LEN(indices);
	tr.lightSphereVolume.numVerts = ARRAY_LEN(positions);
	tr.lightSphereVolume.indexOffset = 0;
	tr.lightSphereVolume.baseVertex = 0;
	tr.lightSphereVolume.program = &tr.deferredPointLightShader;
}

/*
** RE_BeginRegistration
*/
void RE_BeginRegistration( glconfig_t *glconfigOut ) {

	R_Init();

	*glconfigOut = glConfig;

	R_IssuePendingRenderCommands();

	tr.visIndex = 0;
	memset(tr.visClusters, -2, sizeof(tr.visClusters));	// force markleafs to regenerate

	R_ClearFlares();
	RE_ClearScene();

	tr.registered = qtrue;

	// NOTE: this sucks, for some reason the first stretch pic is never drawn
	// without this we'd see a white flash on a level load because the very
	// first time the level shot would not be drawn
//	RE_StretchPic(0, 0, 0, 0, 0, 0, 1, 1, 0);
}

//=============================================================================

void R_SVModelInit()
{
	R_ModelInit();
}

/*
===============
R_ModelInit
===============
*/
void R_ModelInit( void ) {
	model_t		*mod;

	// leave a space for NULL model
	tr.numModels = 0;

	CModelCache->DeleteAll();

	mod = R_AllocModel();
	mod->type = MOD_BAD;

	R_InitBuiltInModels();
}

extern void KillTheShaderHashTable(void);
void RE_HunkClearCrap(void)
{ //get your dirty sticky assets off me, you damn dirty hunk!
	KillTheShaderHashTable();
	tr.numModels = 0;
	CModelCache->DeleteAll();
	tr.numShaders = 0;
	tr.numSkins = 0;
}


/*
================
R_Modellist_f
================
*/
void R_Modellist_f( void ) {
	int		i, j;
	model_t	*mod;
	int		total;
	int		lods;

	total = 0;
	for ( i = 1 ; i < tr.numModels; i++ ) {
		mod = tr.models[i];
		lods = 1;
		for ( j = 1 ; j < MD3_MAX_LODS ; j++ ) {
			if ( mod->data.mdv[j] && mod->data.mdv[j] != mod->data.mdv[j-1] ) {
				lods++;
			}
		}
		ri->Printf( PRINT_ALL, "%8i : (%i) %s\n",mod->dataSize, lods, mod->name );
		total += mod->dataSize;
	}
	ri->Printf( PRINT_ALL, "%8i : Total models\n", total );

#if	0		// not working right with new hunk
	if ( tr.world ) {
		ri->Printf( PRINT_ALL, "\n%8i : %s\n", tr.world->dataSize, tr.world->name );
	}
#endif
}


//=============================================================================


/*
================
R_GetTag
================
*/
static mdvTag_t *R_GetTag( mdvModel_t *mod, int frame, const char *_tagName ) {
	int             i;
	mdvTag_t       *tag;
	mdvTagName_t   *tagName;

	if ( frame >= mod->numFrames ) {
		// it is possible to have a bad frame while changing models, so don't error
		frame = mod->numFrames - 1;
	}

	tag = mod->tags + frame * mod->numTags;
	tagName = mod->tagNames;
	for(i = 0; i < mod->numTags; i++, tag++, tagName++)
	{
		if(!strcmp(tagName->name, _tagName))
		{
			return tag;
		}
	}

	return NULL;
}

void R_GetAnimTag( mdrHeader_t *mod, int framenum, const char *tagName, mdvTag_t * dest)
{
	int				i, j, k;
	int				frameSize;
	mdrFrame_t		*frame;
	mdrTag_t		*tag;

	if ( framenum >= mod->numFrames ) 
	{
		// it is possible to have a bad frame while changing models, so don't error
		framenum = mod->numFrames - 1;
	}

	tag = (mdrTag_t *)((byte *)mod + mod->ofsTags);
	for ( i = 0 ; i < mod->numTags ; i++, tag++ )
	{
		if ( !strcmp( tag->name, tagName ) )
		{
			// uncompressed model...
			//
			frameSize = (intptr_t)( &((mdrFrame_t *)0)->bones[ mod->numBones ] );
			frame = (mdrFrame_t *)((byte *)mod + mod->ofsFrames + framenum * frameSize );

			for (j = 0; j < 3; j++)
			{
				for (k = 0; k < 3; k++)
					dest->axis[j][k]=frame->bones[tag->boneIndex].matrix[k][j];
			}

			dest->origin[0]=frame->bones[tag->boneIndex].matrix[0][3];
			dest->origin[1]=frame->bones[tag->boneIndex].matrix[1][3];
			dest->origin[2]=frame->bones[tag->boneIndex].matrix[2][3];				

			return;
		}
	}

	AxisClear( dest->axis );
	VectorClear( dest->origin );
}

/*
================
R_LerpTag
================
*/
int R_LerpTag( orientation_t *tag, qhandle_t handle, int startFrame, int endFrame, 
					 float frac, const char *tagName ) {
	mdvTag_t	*start, *end;
	mdvTag_t	start_space, end_space;
	int		i;
	float		frontLerp, backLerp;
	model_t		*model;

	model = R_GetModelByHandle( handle );
	if ( !model->data.mdv[0] )
	{
		if(model->type == MOD_MDR)
		{
			start = &start_space;
			end = &end_space;
			R_GetAnimTag((mdrHeader_t *) model->data.mdr, startFrame, tagName, start);
			R_GetAnimTag((mdrHeader_t *) model->data.mdr, endFrame, tagName, end);
		}
		else if( model->type == MOD_IQM ) {
			return R_IQMLerpTag( tag, (iqmData_t *)model->data.iqm,
					startFrame, endFrame,
					frac, tagName );
		} else {

			AxisClear( tag->axis );
			VectorClear( tag->origin );
			return qfalse;

		}
	}
	else
	{
		start = R_GetTag( model->data.mdv[0], startFrame, tagName );
		end = R_GetTag( model->data.mdv[0], endFrame, tagName );
		if ( !start || !end ) {
			AxisClear( tag->axis );
			VectorClear( tag->origin );
			return qfalse;
		}
	}
	
	frontLerp = frac;
	backLerp = 1.0f - frac;

	for ( i = 0 ; i < 3 ; i++ ) {
		tag->origin[i] = start->origin[i] * backLerp +  end->origin[i] * frontLerp;
		tag->axis[0][i] = start->axis[0][i] * backLerp +  end->axis[0][i] * frontLerp;
		tag->axis[1][i] = start->axis[1][i] * backLerp +  end->axis[1][i] * frontLerp;
		tag->axis[2][i] = start->axis[2][i] * backLerp +  end->axis[2][i] * frontLerp;
	}
	VectorNormalize( tag->axis[0] );
	VectorNormalize( tag->axis[1] );
	VectorNormalize( tag->axis[2] );
	return qtrue;
}


/*
====================
R_ModelBounds
====================
*/
void R_ModelBounds( qhandle_t handle, vec3_t mins, vec3_t maxs ) {
	model_t		*model;

	model = R_GetModelByHandle( handle );

	if(model->type == MOD_BRUSH) {
		VectorCopy( model->data.bmodel->bounds[0], mins );
		VectorCopy( model->data.bmodel->bounds[1], maxs );
		
		return;
	} else if (model->type == MOD_MESH) {
		mdvModel_t	*header;
		mdvFrame_t	*frame;

		header = model->data.mdv[0];
		frame = header->frames;

		VectorCopy( frame->bounds[0], mins );
		VectorCopy( frame->bounds[1], maxs );
		
		return;
	} else if (model->type == MOD_MDR) {
		mdrHeader_t	*header;
		mdrFrame_t	*frame;

		header = model->data.mdr;
		frame = (mdrFrame_t *) ((byte *)header + header->ofsFrames);

		VectorCopy( frame->bounds[0], mins );
		VectorCopy( frame->bounds[1], maxs );
		
		return;
	} else if(model->type == MOD_IQM) {
		iqmData_t *iqmData;
		
		iqmData = model->data.iqm;

		if(iqmData->bounds)
		{
			VectorCopy(iqmData->bounds, mins);
			VectorCopy(iqmData->bounds + 3, maxs);
			return;
		}
	}

	VectorClear( mins );
	VectorClear( maxs );
}
