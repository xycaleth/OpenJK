// Filename:-	R_Common.h
//
//  quick and dirty common file that I just paste stuff into to get other model formats to compile
//


#ifndef R_COMMON_H
#define R_COMMON_H


#include "md3_format.h"
#include "mdr_format.h"
#include "mdx_format.h"
#include "glm_code.h"	// tied to mdx_format.h, contains stuff that jake added for render-side


typedef enum {
	RT_MODEL,
	RT_POLY,
	RT_SPRITE,
	RT_BEAM,
	RT_RAIL_CORE,
	RT_RAIL_RINGS,
	RT_LIGHTNING,
	RT_PORTALSURFACE,		// doesn't draw anything, just info for portals
	RT_TERRAIN,				// draws a terrain type entity

	RT_MAX_REF_ENTITY_TYPE
} refEntityType_t;

// search for "R_ModView_AddEntity" for the init code to this struct
typedef struct {
/*
	refEntityType_t	reType;
*/
	int			renderfx;

	qhandle_t	hModel;				// opaque type outside refresh
	mdxaBone_t	tempBoneList[MAX_POSSIBLE_BONES];		// created each frame with a list of all the bones
	surfaceInfo_t *slist;			// pointer to list of surfaces turned off
	boneInfo_t	*blist;				// pointer to list of bones to be overriden
/*
	// most recent data
	vec3_t		lightingOrigin;		// so multi-part models can be lit identically (RF_LIGHTING_ORIGIN)
	float		shadowPlane;		// projection shadows go here, stencils go slightly lower

	vec3_t		axis[3];			// rotation vectors
	qboolean	nonNormalizedAxes;	// axis are not normalized, i.e. they have scale
	float		origin[3];			// also used as MODEL_BEAM's "from"
*/
	int			iFrame_Primary;				// also used as MODEL_BEAM's diameter
/*
	// previous data for frame interpolation
	float		oldorigin[3];		// also used as MODEL_BEAM's "to"
*/
	int			iOldFrame_Primary;
	int			iBoneNum_SecondaryStart;	// -1 if not active
	int			iFrame_Secondary;
	int			iOldFrame_Secondary;
	int			iSurfaceNum_RootOverride;

	float		backlerp;			// 0.0 = current, 1.0 = old
/*
	// texturing
	int			skinNum;			// inline skin index
	qhandle_t	customSkin;			// NULL for default skin
	qhandle_t	customShader;		// use one image for the entire thing

	// misc
	byte		shaderRGBA[4];		// colors used by rgbgen entity shaders
	float		shaderTexCoord[2];	// texture coordinates used by tcMod entity modifiers
	float		shaderTime;			// subtracted from refdef time to control effect start times

	// extra sprite information
	float		radius;
	float		rotation;
*/

	// some stats stuff just for ModView...
	//
	int *piRenderedTris;
	int *piRenderedVerts;
	int *piRenderedSurfs;
	int *piXformedG2Bones;
//	int	*piRenderedBoneWeightsThisSurface;
	int *piRenderedBoneWeights;
	int *piOmittedBoneWeights;

	// some other stuff for modview, I could optimise it more but this is only a viewer...
	//
	mdxaBone_t	*pXFormedG2Bones;
	bool		*pXFormedG2BonesValid;
	mdxaBone_t	*pXFormedG2TagSurfs;
	bool		*pXFormedG2TagSurfsValid;

} refEntity_t;


#include "shader.h"


#define	MAX_SHADERS				1024

#define	MAX_MOD_KNOWN			256	// 1024 (since I only alloc 8 bits to store handle numbers)
#define	MAX_DRAWIMAGES			2048
#define	MAX_LIGHTMAPS			256
#define	MAX_SKINS				1024


#define	MAX_DRAWSURFS			0x10000
#define	DRAWSURF_MASK			(MAX_DRAWSURFS-1)


#define FILE_HASH_SIZE		1024


typedef struct {
	vec3_t		origin;
	vec3_t		axis[3];
} orientation_t;



// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum {
	PRINT_ALL,
	PRINT_DEVELOPER,		// only print when "developer 1"
	PRINT_WARNING,
	PRINT_ERROR
} printParm_t;



typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_DISCONNECT,				// don't kill server
	ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;


// any changes in surfaceType must be mirrored in rb_surfaceTable[]

typedef enum {
	SF_BAD,
	SF_SKIP,				// ignore
	SF_FACE,
	SF_GRID,
	SF_TRIANGLES,
	SF_POLY,
	SF_MD3,
	SF_MD4,
	SF_MDX,
	SF_FLARE,
	SF_ENTITY,				// beams, rails, lightning, etc that can be determined by entity
	SF_DISPLAY_LIST,

	SF_NUM_SURFACE_TYPES,
	SF_MAX = 0xffffffff			// ensures that sizeof( surfaceType_t ) == sizeof( int )
} surfaceType_t;


typedef struct drawSurf_s {
	unsigned			sort;			// bit combination for fast compares
	surfaceType_t		*surface;		// any of surface*_t
} drawSurf_t;


typedef struct msurface_s {
	int					viewCount;		// if == tr.viewCount, already added
	struct shader_s		*shader;
	int					fogIndex;

	surfaceType_t		*data;			// any of srf*_t
} msurface_t;



typedef struct {
	vec3_t		bounds[2];		// for culling
	msurface_t	*firstSurface;
	int			numSurfaces;
} bmodel_t;


typedef struct model_s {
	char		name[MAX_QPATH];
	modtype_t	type;
	int			index;				// model = tr.models[model->index]

	int			dataSize;			// just for listing purposes

	union
	{
		bmodel_t	*bmodel;			// only if type == MOD_BRUSH
		md3Header_t	*md3[MD3_MAX_LODS];	// only if type == MOD_MESH
		md4Header_t	*md4;				// only if type == MOD_MD4
		mdxmHeader_t *mdxm;				// only if type == MOD_GL2M which is a GHOUL II Mesh file NOT a GHOUL II animation file
		mdxaHeader_t *mdxa;				// only if type == MOD_GL2A which is a GHOUL II Animation file
		void *pvData;	// give common addressing to higher functions that don't care about structs
	};
	mdxmSurface_t	*mdxmsurf[MAX_G2_LODS][MAX_G2_SURFACES];
	int			 numLods;

} model_t;




// a trRefEntity_t has all the information passed in by
// the client game, as well as some locally derived info
typedef struct {
	refEntity_t	e;
} trRefEntity_t;

// trRefdef_t holds everything that comes in refdef_t,
// as well as the locally generated scene information
typedef struct {	
	int			num_entities;
	trRefEntity_t	entities[MAX_MOD_KNOWN];	//	// MODVIEWHACK	// trRefEntity_t	*entities;

	int			numDrawSurfs;
	struct drawSurf_s	drawSurfs[MAX_DRAWSURFS];	//MODVIEWHACK	//*drawSurfs;
} trRefdef_t;


/*
** trGlobals_t 
**
** Most renderer globals are defined here.
** backend functions should never modify any of these fields,
** but may read fields that aren't dynamically modified
** by the frontend.
*/
typedef struct {
	trRefEntity_t			*currentEntity;
	int						currentEntityNum;	// used during for-next loop to add all ent surface
	model_t					*currentModel;

	trRefdef_t				refdef;

	model_t					*models[MAX_MOD_KNOWN];
	int						numModels;
} trGlobals_t;


//
// these are the functions imported by the refresh module
//
typedef struct
{
	// print message on the local console
	void	(QDECL *Printf)( int printLevel, const char *fmt, ...);
	// abort the game  (flushes model now instead!)
	void	(QDECL *Error)( int errorLevel, const char *fmt, ...);
/*

	// functions used to decode key/value pairs
	char	*(QDECL *ValueForKey)(const char *input, const char *key);

	// milliseconds should only be used for profiling, never
	// for anything game related.  Get time from the refdef
	int		(*Milliseconds)( void );

	// stack based memory allocation for per-level things that
	// won't be freed
	void	(*Hunk_Clear)( void );
*/
	void	*(*Hunk_Alloc)( int size );

	void	*(*Hunk_AllocateTempMemory)( int size );
	void	(*Hunk_FreeTempMemory)( void *block );

	// dynamic memory allocator for things that need to be freed
	void	*(*Malloc)( int bytes );
	void	(*Free)( void *buf );
/*
	cvar_t	*(*Cvar_Get)( const char *name, const char *value, int flags );
	void	(*Cvar_Set)( const char *name, const char *value );

	void	(*Cmd_AddCommand)( const char *name, void(*cmd)(void) );
	void	(*Cmd_RemoveCommand)( const char *name );

	int		(*Cmd_Argc) (void);
	char	*(*Cmd_Argv) (int i);

	void	(*Cmd_ExecuteText) (int exec_when, const char *text);

	// visualization for debugging collision detection
	void	(*CM_DrawDebugSurface)( void (*drawPoly)(int color, int numPoints, float *points) );

	// a -1 return means the file does not exist
	// NULL can be passed for buf to just determine existance
	int		(*FS_FileIsInPAK)( const char *name, int *pCheckSum );
*/
	int		(*FS_ReadFile)( const char *name, void **buf );
	void	(*FS_FreeFile)( void *buf );
/*
	char **	(*FS_ListFiles)( const char *name, const char *extension, int *numfilesfound );
	void	(*FS_FreeFileList)( char **filelist );
*/
	int 	(*FS_WriteFile)( const char *qpath, const void *buffer, int size );

} refimport_t;



// renderfx flags
#define	RF_MINLIGHT			1		// allways have some light (viewmodel, some items)
#define	RF_THIRD_PERSON		2		// don't draw through eyes, only mirrors (player bodies, chat sprites)
#define	RF_FIRST_PERSON		4		// only draw through eyes (view weapon, damage blood blob)
#define	RF_DEPTHHACK		8		// for view weapon Z crunching
#define	RF_NOSHADOW			64		// don't add stencil shadows

#define RF_LIGHTING_ORIGIN	128		// use refEntity->lightingOrigin instead of refEntity->origin
									// for lighting.  This allows entities to sink into the floor
									// with their origin going solid, and allows all parts of a
									// player to get the same lighting
#define	RF_SHADOW_PLANE		256		// use refEntity->shadowPlane
#define	RF_WRAP_FRAMES		512		// mod the model frames by the maxframes to allow continuous
									// animation without needing to know the frame count

#define	RF_CAP_FRAMES		1024	// cap the model frames by the maxframes for one shot anims


extern trGlobals_t tr;
extern refimport_t ri;


#include "R_MD3.h"
#include "R_MDR.h"
#include "R_GLM.h"
#include "shader.h"
#include "R_Surface.h"



//////////////////////////////////////////////////////////
//
// some crap for compile-ease...
#define Q_stricmp	stricmp
#define Q_strlwr	strlwr
#define LittleLong(x) x
#define LittleShort(x) x
#define LittleFloat(x) x
#define R_SyncRenderThread()
#define Com_Error ri.Error
void Q_strncpyz( char *dest, LPCSTR src, int destlen);
float Com_Clamp( float min, float max, float value );
//
// some generic import functions... (all in R_MODEL.H)
//
char *COM_SkipPath (char *pathname);
void COM_StripExtension( const char *in, char *out );
void COM_DefaultExtension (char *path, int maxSize, const char *extension );
void QDECL Com_sprintf( char *dest, int size, const char *fmt, ...);
int    LongSwap (int l);
#define BigLong(x) LongSwap(x)
void Com_Printf( const char *format, ... );
long generateHashValue( const char *fname );
model_t	*R_GetModelByHandle( qhandle_t index );
void R_AddDrawSurf( surfaceType_t *surface, GLuint gluiTextureBind);
void R_DecomposeSort( unsigned sort, int *entityNum, GLuint* gluiTextureBind);
int  R_ComputeLOD( trRefEntity_t *ent );
//
// other crap...
//
#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))
#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define Vector4Copy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define	SnapVector(v) {v[0]=(int)v[0];v[1]=(int)v[1];v[2]=(int)v[2];}
void AxisClear( vec3_t axis[3] );
//
//////////////////////////////////////////////////////////


#endif	// #ifndef R_COMMON_H


///////////////// eof //////////////////

