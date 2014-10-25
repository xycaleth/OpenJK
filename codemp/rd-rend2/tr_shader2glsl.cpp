#include "tr_local.h"

#include <sstream>
#include <string>
/*
struct shader_s:
	-- not sure about this yet
	int			lightmapIndex[MAXLIGHTMAPS];	// for a shader to match, both name and all lightmapIndex must match
	byte		styles[MAXLIGHTMAPS];

	-- Possibly useful?
	float		sort;					// lower numbered shaders draw before higher numbered

	-- not useful
	int			surfaceFlags;			// if explicitlyDefined, this will have SURF_* flags
	int			contentFlags;

	-- not useful
	qboolean	isSky;
	skyParms_t	sky;

	-- cbuffer data
	fogParms_t	fogParms;
	float		portalRange;			// distance to fog out at
	int			multitextureEnv;		// 0, GL_MODULATE, GL_ADD (FIXME: put in stage)

	-- rasterizer state
	cullType_t	cullType;				// CT_FRONT_SIDED, CT_BACK_SIDED, or CT_TWO_SIDED
	qboolean	polygonOffset;			// set for decals and other items that must be offset 

	-- sampler object state
	qboolean	noMipMaps;				// for console fonts, 2D elements, etc.
	qboolean	noPicMip;				// for images that must always be full resolution
	qboolean	noTC;					// for images that don't want to be texture compressed (eg skies)

	-- rasterizer state
	fogPass_t	fogPass;				// draw a blended pass, possibly with depth test equals

	-- vertex shader
	int         vertexAttribs;          // not all shaders will need all data to be gathered

	-- vertex shader
	int			numDeforms;
	deformStage_t	deforms[MAX_SHADER_DEFORMS];

	struct deformStage_t:
		-- vertex shader
		deform_t	deformation;			// vertex coordinate modification type

		-- rest is cbuffer data
		vec3_t		moveVector;
		waveForm_t	deformationWave;
		float		deformationSpread;

		float		bulgeWidth;
		float		bulgeHeight;
		float		bulgeSpeed;

	int			numUnfoggedPasses;
	shaderStage_t	*stages[MAX_SHADER_STAGES];	
	
	struct shaderStage_t:
		-- not useful?
		qboolean		isDetail;

		-- fragment shader
		qboolean		glow;
	
		textureBundle_t	bundle[NUM_TEXTURE_BUNDLES];

		struct textureBundle_t:
			image_t			*image[MAX_IMAGE_ANIMATIONS];
			int				numImageAnimations;
			float			imageAnimationSpeed;

			texCoordGen_t	tcGen;
			vec3_t			tcGenVectors[2];

			int				numTexMods;
			texModInfo_t	*texMods;

			int				videoMapHandle;
			qboolean		isLightmap;
			qboolean		oneShotAnimMap;
			qboolean		isVideoMap;

		waveForm_t		rgbWave;
		colorGen_t		rgbGen;

		waveForm_t		alphaWave;
		alphaGen_t		alphaGen;

		byte			constantColor[4];			// for CGEN_CONST and AGEN_CONST

		-- rasterizer state
		uint32_t		stateBits;					// GLS_xxxx mask

		acff_t			adjustColorsForFog;

		int				lightmapStyle;

		stageType_t     type;

		-- not useful
		struct shaderProgram_s *glslShaderGroup;
		int glslShaderIndex;

		-- cbuffer data
		vec4_t normalScale;
		vec4_t specularScale;

		qboolean		isSurfaceSprite;

	-- cbuffer data
	float clampTime;                                  // time this shader is clamped to
	float timeOffset;                                 // current time offset for this shader

*/

// Make sure to update this array if any ATTR_ enums are added.
static struct AttributeMeta
{
	int type;
	const char *name;
} attributeMeta[] = {
	{ GLSL_VEC3, "attr_Position" },
	{ GLSL_VEC2, "attr_TexCoord0" },
	{ GLSL_VEC2, "attr_TexCoord1" },
	{ GLSL_VEC4, "attr_Tangent" },
	{ GLSL_VEC3, "attr_Normal" },
	{ GLSL_VEC4, "attr_Color" },
	{ GLSL_VEC4, "attr_PaintColor" },
	{ GLSL_VEC3, "attr_LightDirection" },
	{ GLSL_VEC4, "attr_BoneIndexes" },
	{ GLSL_VEC4, "attr_BoneWeights" },
	{ GLSL_VEC3, "attr_Position2" },
	{ GLSL_VEC4, "attr_Tangent2" },
	{ GLSL_VEC3, "attr_Normal2" }
};

static const char *deformTypeStrings[] = {
	"DEFORM_NONE",
	"DEFORM_WAVE",
	"DEFORM_NORMALS",
	"DEFORM_BULGE",
	"DEFORM_MOVE",
	"DEFORM_PROJECTION_SHADOW"
};

static const char *waveFormStrings[] = {
	"WF_NONE",
	"WF_SIN",
	"WF_SQUARE",
	"WF_TRIANGLE",
	"WF_SAWTOOTH",
	"WF_INVERSE_SAWTOOTH",
};

static const char *glslTypeStrings[] = {
	"int",
	"sampler2D",
	"samplerCube",
	"float",
	"vec2",
	"vec3",
	"vec4",
	"mat4"
};

static const char *genericShaderDefStrings[] = {
	"GENERICDEF_USE_DEFORM_VERTEXES",
	"GENERICDEF_USE_TCGEN_AND_TCMOD",
	"GENERICDEF_USE_VERTEX_ANIMATION",
	"GENERICDEF_USE_FOG",
	"GENERICDEF_USE_RGBAGEN",
	"GENERICDEF_USE_LIGHTMAP",
	"GENERICDEF_USE_SKELETAL_ANIMATION",
	"GENERICDEF_USE_GLOW_BUFFER",
};

static uniform_t bundleToUniform[NUM_TEXTURE_BUNDLES] = {
	UNIFORM_DIFFUSEMAP,
	UNIFORM_LIGHTMAP,
	UNIFORM_NORMALMAP,
	UNIFORM_DELUXEMAP,
	UNIFORM_SPECULARMAP,
	UNIFORM_SHADOWMAP,
	UNIFORM_CUBEMAP
};

/*
Eventually, we want to render like this:

for each view:
  for each object:
    update cbuffer

for each view:
  for each object:
    ensure shaders loaded

for each view:
  for each object:
    draw object
*/

/*
Runtime group variations:
animation: {no animation, skeletal animation, vertex animation}
fog: {no fog, with fog}
render pass: {depth prepass, shadow, render} <-- depth prepass and shadow the same thing?
*/

static void SetBit( uint32_t *bitfieldArray, int bit )
{
	int index = bit / 32;
	int elementBit = bit - index * 32;

	bitfieldArray[index] |= (1 << elementBit);
}

static void ClearBit( uint32_t *bitfieldArray, int bit )
{
	int index = bit / 32;
	int elementBit = bit - index * 32;

	bitfieldArray[index] &= ~(1 << elementBit);
}

static bool IsBitSet( uint32_t *bitfieldArray, int bit )
{
	int index = bit / 32;
	int elementBit = bit - index * 32;

	return (bitfieldArray[index] & (1 << elementBit)) != 0;
}

struct GLSLShader
{
	std::ostringstream code;

	uint32_t attributes;
	int samplersArraySizes[NUM_TEXTURE_BUNDLES];
	int uniformsArraySizes[UNIFORM_COUNT];

	image_t *textures[MAX_SHADER_STAGES * NUM_TEXTURE_BUNDLES];
};

struct ConstantDescriptor
{
	int uniform;
	int type;
	int offset;
	int location;
	int numElementsInBaseType;
	int totalNumElements;
};

struct SamplerDescriptor
{
	GLenum target;
};

struct ShaderProgram
{
	ShaderProgram *next;

	uint32_t permutation;
	bool used;

	int vertexShader;
	int fragmentShader;
	int program;

	int numSamplers;
	SamplerDescriptor *samplers;

	int numUniforms;
	int constantsBufferSizeInBytes;
	ConstantDescriptor *constants;

	// Mapping from UNIFORM_* -> array index into 'constants' array.
	int uniformDataIndices[UNIFORM_COUNT];

	// Mapping from {stage, bundle} -> sampler index
	int samplerIndexes[MAX_SHADER_STAGES][NUM_TEXTURE_BUNDLES];
};

struct Material
{
	ShaderProgram *program;

	void *constantData;
	GLuint *textures;
};

int numQ3ShadersLoaded;
int numGLSLPrograms;

void GLSLGeneratorInitContext( GLSLGeneratorContext *ctx )
{
	memset( ctx->shaderProgramsTable, 0, sizeof( ctx->shaderProgramsTable ) );
}

void GLSLGeneratorDestroyContext( GLSLGeneratorContext *ctx )
{
	for ( int i = 0; i < GLSLGeneratorContext::MAX_SHADER_PROGRAM_TABLE_SIZE; i++ )
	{
		ShaderProgram *program = ctx->shaderProgramsTable[i];

		while ( program != NULL )
		{
			ShaderProgram *next = program->next;

			qglDeleteProgram( program->program );

			delete[] program->constants;
			delete[] program->samplers;
			delete program;

			program = next;
		}
	}
}

static bool AddShaderHeaderCode( const shader_t *shader, const char *defines[], uint32_t permutation, std::ostream& code )
{
	// Add permutations for debug
	code << "// " << shader->name << '\n';

	if ( permutation == 0 )
	{
		code << "// No shader flags\n";
	}
	else
	{
		for ( int i = 0, j = 1; i < ARRAY_LEN(genericShaderDefStrings); i++, j <<= 1 )
		{
			if ( permutation & j )
			{
				code << "// " << genericShaderDefStrings[i] << '\n';
			}
		}

		if ( permutation & GENERICDEF_USE_DEFORM_VERTEXES )
		{
			for ( int i = 0 ; i < ARRAY_LEN( deformTypeStrings ); i++ )
			{
				code << "#define " << deformTypeStrings[i] << ' ' << i << '\n';
			}

			for ( int i = 0 ; i < ARRAY_LEN( waveFormStrings ); i++ )
			{
				code << "#define " << waveFormStrings[i] << ' ' << i << '\n';
			}
		}
	}

	if ( defines != NULL )
	{
		for ( const char **define = defines; *define != NULL; define += 2 )
		{
			code << "#define " << define[0] << ' ' << define[1] << '\n';
		}
	}

	code << "#line 0\n";
	code << "#version 150 core\n";

	return true;
}

static void GenerateGenericVertexShaderCode(
	const shader_t *shader,
	const char *defines[],
	uint32_t permutation,
	int samplerMapping[MAX_SHADER_STAGES][NUM_TEXTURE_BUNDLES],
	GLSLShader& genShader )
{
	std::ostream& code = genShader.code;
	uint32_t attributes = shader->vertexAttribs;

	// Determine inputs needed
	if ( permutation & GENERICDEF_USE_VERTEX_ANIMATION )
	{
		attributes |= ATTR_POSITION2;
		attributes |= ATTR_NORMAL2;
	}
	else if ( permutation & GENERICDEF_USE_SKELETAL_ANIMATION )
	{
		attributes |= ATTR_BONE_INDEXES;
		attributes |= ATTR_BONE_WEIGHTS;
	}

	if ( permutation & GENERICDEF_USE_RGBAGEN )
	{
		attributes |= ATTR_COLOR;
	}

	if ( permutation & (GENERICDEF_USE_TCGEN_AND_TCMOD | GENERICDEF_USE_RGBAGEN) )
	{
		attributes |= ATTR_TEXCOORD1;
	}

	// Determine uniforms needed
	memset( genShader.uniformsArraySizes, 0, sizeof( genShader.uniformsArraySizes ) );
	memset( genShader.samplersArraySizes, 0, sizeof( genShader.samplersArraySizes ) );

	genShader.uniformsArraySizes[UNIFORM_MODELVIEWPROJECTIONMATRIX] = 1;
	genShader.uniformsArraySizes[UNIFORM_BASECOLOR] = 1;
	genShader.uniformsArraySizes[UNIFORM_VERTCOLOR] = 1;
	
	if ( permutation & (GENERICDEF_USE_TCGEN_AND_TCMOD | GENERICDEF_USE_RGBAGEN) )
	{
		genShader.uniformsArraySizes[UNIFORM_LOCALVIEWORIGIN] = 1;
	}

	if ( permutation & GENERICDEF_USE_TCGEN_AND_TCMOD )
	{
		// Per unique diffuse tex mods
		genShader.uniformsArraySizes[UNIFORM_DIFFUSETEXMATRIX] = 1;
		genShader.uniformsArraySizes[UNIFORM_DIFFUSETEXOFFTURB] = 1;

		// Per unique tc gens
		genShader.uniformsArraySizes[UNIFORM_TCGEN0] = 1;
		genShader.uniformsArraySizes[UNIFORM_TCGEN0VECTOR0] = 1;
		genShader.uniformsArraySizes[UNIFORM_TCGEN0VECTOR1] = 1;
	}

	if ( permutation & GENERICDEF_USE_FOG )
	{
		genShader.uniformsArraySizes[UNIFORM_FOGCOLORMASK] = 1;
		genShader.uniformsArraySizes[UNIFORM_FOGDEPTH] = 1;
		genShader.uniformsArraySizes[UNIFORM_FOGDISTANCE] = 1;
		genShader.uniformsArraySizes[UNIFORM_FOGEYET] = 1;
	}

	if ( permutation & GENERICDEF_USE_DEFORM_VERTEXES )
	{
		// Per deform vertex entry
		genShader.uniformsArraySizes[UNIFORM_DEFORMTYPE] = 1;
		genShader.uniformsArraySizes[UNIFORM_DEFORMFUNC] = 1;
		genShader.uniformsArraySizes[UNIFORM_DEFORMPARAMS] = 1;
		genShader.uniformsArraySizes[UNIFORM_TIME] = 1;

		genShader.uniformsArraySizes[UNIFORM_DEFORMFUNC] = shader->numDeforms;
		genShader.uniformsArraySizes[UNIFORM_DEFORMTYPE] = shader->numDeforms;
		genShader.uniformsArraySizes[UNIFORM_DEFORMPARAMS] = shader->numDeforms;
	}

	if ( permutation & GENERICDEF_USE_RGBAGEN )
	{
		// Per stage?
		genShader.uniformsArraySizes[UNIFORM_COLORGEN] = 1;
		genShader.uniformsArraySizes[UNIFORM_ALPHAGEN] = 1;
		genShader.uniformsArraySizes[UNIFORM_AMBIENTLIGHT] = 1;
		genShader.uniformsArraySizes[UNIFORM_DIRECTEDLIGHT] = 1;
		genShader.uniformsArraySizes[UNIFORM_MODELLIGHTDIR] = 1;
		genShader.uniformsArraySizes[UNIFORM_PORTALRANGE] = 1;
	}

	if ( permutation & GENERICDEF_USE_SKELETAL_ANIMATION )
	{
		genShader.uniformsArraySizes[UNIFORM_BONE_MATRICES] = 1;
	}
	else if ( permutation & GENERICDEF_USE_VERTEX_ANIMATION )
	{
		genShader.uniformsArraySizes[UNIFORM_VERTEXLERP] = 1;
	}

	// Generate the code
	AddShaderHeaderCode( shader, defines, permutation, code );

	code << '\n';

	// Add uniforms
	for ( int i = 0; i < UNIFORM_COUNT; i++ )
	{
		if ( genShader.uniformsArraySizes[i] > 0 )
		{
			code << "uniform " << glslTypeStrings[uniformsInfo[i].type] << ' ' << uniformsInfo[i].name;
			code << '[' << (uniformsInfo[i].size * genShader.uniformsArraySizes[i]) << ']';
			code << ";\n";
		}
	}

	code << '\n';

	//
	// Add inputs
	//
	assert( sizeof( int ) == 4 );
	for ( int i = 0, j = 1; i < 32; i++, j <<= 1 )
	{
		if ( attributes & j )
		{
			code << "in " << glslTypeStrings[attributeMeta[i].type] << ' ' << attributeMeta[i].name << ";\n";
		}
	}

	code << '\n';

	// 
	// Add outputs
	//
	if ( shader->numUnfoggedPasses > 0 )
	{
		// Fog shaders don't have any unfogged passes

		code << "out vec2 var_TexCoords[" << shader->numUnfoggedPasses << "];\n";

		if ( permutation & GENERICDEF_USE_RGBAGEN )
		{
			code << "out vec4 var_Colors[" << shader->numUnfoggedPasses << "];\n";
		}
	}

	if ( permutation & GENERICDEF_USE_LIGHTMAP )
	{
		code << "out vec2 var_LightTex;\n";
	}

	code << '\n';

	// Helper functions
	if ( permutation & GENERICDEF_USE_DEFORM_VERTEXES )
	{
		code << "float GetNoiseValue( float x, float y, float z, float t )\n\
{\n\
	return fract( sin( dot( vec4( x, y, z, t ), vec4( 12.9898, 78.233, 12.9898, 78.233 ) )) * 43758.5453 );\n\
}\n\
\n\
float CalculateDeformScale( in int func, in float time, in float phase, in float frequency )\n\
{\n\
	float value = phase + time * frequency;\n\
\n\
	switch ( func )\n\
	{\n\
		case WF_SIN:\n\
			return sin(value * 2.0 * 3.14159);\n\
		case WF_SQUARE:\n\
			return sign(0.5 - fract(value));\n\
		case WF_TRIANGLE:\n\
			return abs(fract(value + 0.75) - 0.5) * 4.0 - 1.0;\n\
		case WF_SAWTOOTH:\n\
			return fract(value);\n\
		case WF_INVERSE_SAWTOOTH:\n\
			return 1.0 - fract(value);\n\
		default:\n\
			return 0.0;\n\
	}\n\
}\n\
\n\
vec3 DeformPosition(\n\
	const vec3 pos,\n\
	const vec3 normal,\n\
	const vec2 st,\n\
	in float time,\n\
	in int type,\n\
	in int waveFunc,\n\
	in float param0,\n\
	in float param1,\n\
	in float param2,\n\
	in float param3,\n\
	in float param4,\n\
	in float param5,\n\
	in float param6\n\
)\n\
{\n\
	switch ( type )\n\
	{\n\
		default: return pos;\n\
		\n\
		case DEFORM_BULGE:\n\
		{\n\
			float bulgeHeight = param1;\n\
			float bulgeWidth = param2;\n\
			float bulgeSpeed = param3;\n\
			\n\
			float scale = CalculateDeformScale( WF_SIN, time, bulgeWidth * st.x, bulgeSpeed );\n\
			\n\
			return pos + normal * scale * bulgeHeight;\n\
		}\n\
\n\
		case DEFORM_WAVE:\n\
		{\n\
			float base = param0;\n\
			float amplitude = param1;\n\
			float phase = param2;\n\
			float frequency = param3;\n\
			float spread = param4;\n\
\n\
			float offset = dot( pos.xyz, vec3( spread ) );\n\
			float scale = CalculateDeformScale( waveFunc, time, phase + offset, frequency );\n\
\n\
			return pos + normal * (base + scale * amplitude);\n\
		}\n\
\n\
		case DEFORM_MOVE:\n\
		{\n\
			float base = param0;\n\
			float amplitude = param1;\n\
			float phase = param2;\n\
			float frequency = param3;\n\
			vec3 direction = vec3( param4, param5, param6 );\n\
\n\
			float scale = CalculateDeformScale( waveFunc, time, phase, frequency );\n\
\n\
			return pos + direction * (base + scale * amplitude);\n\
		}\n\
\n\
		case DEFORM_PROJECTION_SHADOW:\n\
		{\n\
			vec3 ground = vec3(\n\
				param0,\n\
				param1,\n\
				param2);\n\
			float groundDist = param3;\n\
			vec3 lightDir = vec3(\n\
				param4,\n\
				param5,\n\
				param6);\n\
\n\
			float d = dot( lightDir, ground );\n\
\n\
			lightDir = lightDir * max( 0.5 - d, 0.0 ) + ground;\n\
			d = 1.0 / dot( lightDir, ground );\n\
\n\
			vec3 lightPos = lightDir * d;\n\
\n\
			return pos - lightPos * dot( pos, ground ) + groundDist;\n\
		}\n\
	}\n\
}\n\
\n\
vec3 DeformNormal(\n\
	in const vec3 position,\n\
	in const vec3 normal,\n\
	in float time,\n\
	in int type,\n\
	in float amplitude,\n\
	in float frequency\n\
)\n\
{\n\
	if ( type != DEFORM_NORMALS )\n\
	{\n\
		return normal;\n\
	}\n\
\n\
	vec3 outNormal = normal;\n\
	const float scale = 0.98;\n\
	\n\
	outNormal.x += amplitude * GetNoiseValue(\n\
		position.x * scale,\n\
		position.y * scale,\n\
		position.z * scale,\n\
		time * frequency );\n\
\n\
	outNormal.y += amplitude * GetNoiseValue(\n\
		100.0 * position.x * scale,\n\
		position.y * scale,\n\
		position.z * scale,\n\
		time * frequency );\n\
\n\
	outNormal.z += amplitude * GetNoiseValue(\n\
		200.0 * position.x * scale,\n\
		position.y * scale,\n\
		position.z * scale,\n\
		time * frequency );\n\
\n\
	return outNormal;\n\
}\n";
	}

	if ( permutation & GENERICDEF_USE_TCGEN_AND_TCMOD )
	{
		code << "vec2 GenTexCoords(in int TCGen, in vec3 position, in vec3 normal, in vec3 TCGenVector0, in vec3 TCGenVector1)\n\
{\n\
	vec2 tex = attr_TexCoord0;\n\
	\n\
	if (TCGen >= 2 && TCGen <= 4)\n\
	{\n\
		tex = attr_TexCoord1;\n\
	}\n\
	else if (TCGen == 7)\n\
	{\n\
		vec3 viewer = normalize(u_LocalViewOrigin[0] - position);\n\
		vec2 ref = reflect(viewer, normal).yz;\n\
		tex.s = ref.x * -0.5 + 0.5;\n\
		tex.t = ref.y *  0.5 + 0.5;\n\
	}\n\
	else if (TCGen == 9)\n\
	{\n\
		tex = vec2(dot(position, TCGenVector0), dot(position, TCGenVector1));\n\
	}\n\
	\n\
	return tex;\n\
}\n\n";

		code << "vec2 ModTexCoords(in vec2 st, in vec3 position, in vec4 texMatrix, in vec4 offTurb)\n\
{\n\
	float amplitude = offTurb.z;\n\
	float phase = offTurb.w * 2.0 * 3.14159;\n\
	vec2 st2;\n\
	st2.x = st.x * texMatrix.x + (st.y * texMatrix.z + offTurb.x);\n\
	st2.y = st.x * texMatrix.y + (st.y * texMatrix.w + offTurb.y);\n\
	\n\
	vec2 offsetPos = vec2(position.x + position.z, position.y);\n\
	\n\
	vec2 texOffset = sin(offsetPos * (2.0 * 3.14159 / 1024.0) + vec2(phase));\n\
	\n\
	return st2 + texOffset * amplitude;\n\
}\n\n";
	}

	if ( permutation & GENERICDEF_USE_FOG )
	{
		code << "float CalcFog(in vec3 position)\n\
{\n\
	float s = dot(vec4(position, 1.0), u_FogDistance[0]) * 8.0;\n\
	float t = dot(vec4(position, 1.0), u_FogDepth[0]);\n\
	\n\
	float eyeOutside = float(u_FogEyeT[0] < 0.0);\n\
	float fogged = float(t < eyeOutside);\n\
	\n\
	t += 1e-6;\n\
	t *= fogged / (t - u_FogEyeT[0] * eyeOutside);\n\
	\n\
	return s * t;\n\
}\n\n";
	}

	// Main function body
	code << "void main() {\n";

	if ( permutation & GENERICDEF_USE_VERTEX_ANIMATION )
	{
		code << "	vec3 position = mix(attr_Position, attr_Position2, u_VertexLerp[0]);\n";
		code << "	vec3 normal = mix(attr_Normal, attr_Normal2, u_VertexLerp[0]);\n";
		code << "	normal = normalize(normal - vec3(0.5));\n";
	}
	else if ( permutation & GENERICDEF_USE_SKELETAL_ANIMATION )
	{
		code << "	vec4 position4 = vec4(0.0);\n\
	vec4 normal4 = vec4(0.0);\n\
	vec4 originalPosition = vec4(attr_Position, 1.0);\n\
	vec4 originalNormal = vec4(attr_Normal - vec3 (0.5), 0.0);\n\
	\n\
	for ( int i = 0; i < 4; i++ )\n\
	{\n\
		int boneIndex = int(attr_BoneIndexes[i]);\n\
	\n\
		position4 += (u_BoneMatrices[boneIndex] * originalPosition) * attr_BoneWeights[i];\n\
		normal4 += (u_BoneMatrices[boneIndex] * originalNormal) * attr_BoneWeights[i];\n\
	}\n\
	\n\
	vec3 position = position4.xyz;\n\
	vec3 normal = normalize(normal4.xyz);\n";
	}
	else
	{
		code << "	vec3 position = attr_Position;\n";
		code << "	vec3 normal = attr_Normal * 2.0 - vec3(1.0);\n";
	}

	if ( permutation & GENERICDEF_USE_DEFORM_VERTEXES )
	{
		code << '\n';

		for ( int i = 0; i < shader->numDeforms; i++ )
		{
			code << "	position = DeformPosition(\n\
		position,\n\
		normal,\n\
		attr_TexCoord0.st,\n\
		u_Time[0],\n\
		u_DeformType[" << i << "],\n\
		u_DeformFunc[" << i << "],\n";

			for ( int j = 0; j < 7; j++ )
			{
				code << "		u_DeformParams[" << (i * 7 + j) << "]";

				if ( j < 6 )
				{
					code << ',';
				}

				code << '\n';
			}

			code << "	);\n";

			code << "	normal = DeformNormal(\n\
		position,\n\
		normal,\n\
		u_Time[0],\n\
		u_DeformType[" << i << "],\n";

			// u_DeformParams[1]
			code << "		u_DeformParams[" << (i * 7 + 1) << "],\n";

			// u_DeformParams[3]
			code << "		u_DeformParams[" << (i * 7 + 3) << "]);\n";
		}
	}

	code << "\n\
	gl_Position = u_ModelViewProjectionMatrix[0] * vec4(position, 1.0);\n\
	\n\
	vec2 tex;\n";

	for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
	{
		const shaderStage_t *stage = shader->stages[i];

		code << "	var_TexCoords[" << i << "] = attr_TexCoord0;\n";
	}

	if ( permutation & GENERICDEF_USE_LIGHTMAP )
	{
		code << "	var_LightTex = attr_TexCoord1;\n";
	}

	if ( shader->numUnfoggedPasses > 0 )
	{
		if ( permutation & GENERICDEF_USE_RGBAGEN )
		{
			code << '\n';
			for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
			{
				code << "	var_Colors[" << i << "] = u_VertColor[0] * attr_Color + u_BaseColor[0];\n";
			}
		}
	}

	code << "}\n\n";
}

// We don't clamp the output here. Do we need to?
static void AddBlendEquationCode( std::ostream& code, uint32_t state )
{
	code << "	dst = ";

	switch ( state & GLS_SRCBLEND_BITS )
	{
		case GLS_SRCBLEND_ALPHA_SATURATE:
			assert(!"Not used?");
			break;
		case GLS_SRCBLEND_DST_ALPHA:
			code << "dst.a";
			break;
		case GLS_SRCBLEND_DST_COLOR:
			code << "vec4(dst.rgb, 1.0)";
			break;
		case GLS_SRCBLEND_ONE:
			code << "1.0";
			break;
		case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
			code << "(1.0 - dst.a)";
			break;
		case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
			code << "(1.0 - vec4(dst.rgb, 1.0))";
			break;
		case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
			code << "(1.0 - src.a)";
			break;
		case GLS_SRCBLEND_SRC_ALPHA:
			code << "src.a";
			break;
		case GLS_SRCBLEND_ZERO:
			code << "0.0";
			break;
		default:
			code << "1.0";
			break;
	}

	code << " * src + dst * ";

	switch ( state & GLS_DSTBLEND_BITS )
	{
		case GLS_DSTBLEND_DST_ALPHA:
			code << "dst.a";
			break;
		case GLS_DSTBLEND_ONE:
			code << "1.0";
			break;
		case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
			code << "(1.0 - dst.a)";
			break;
		case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
			code << "(1.0 - src.a)";
			break;
		case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
			code << "(1.0 - vec4(src.rgb, 1.0))";
			break;
		case GLS_DSTBLEND_SRC_COLOR:
			code << "vec4(src.rgb, 1.0)";
			break;
		case GLS_DSTBLEND_SRC_ALPHA:
			code << "src.a";
			break;
		case GLS_DSTBLEND_ZERO:
			code << "0.0";
			break;
		default:
			code << "0.0";
			break;
	}

	code << ";\n";
}

static int GetTextureCount( int samplerMapping[MAX_SHADER_STAGES][NUM_TEXTURE_BUNDLES], int bundleType, int numStages )
{
	int count = 0;
	for ( int i = 0; i < numStages; i++ )
	{
		if ( samplerMapping[i][bundleType] != -1 )
		{
			count++;
		}
	}

	return count;
}

static void GenerateGenericFragmentShaderCode(
	const shader_t *shader,
	const char *defines[],
	uint32_t permutation,
	int samplerMapping[MAX_SHADER_STAGES][NUM_TEXTURE_BUNDLES],
	GLSLShader& genShader )
{
	std::ostream& code = genShader.code;

	// Determine uniforms needed
	memset( genShader.uniformsArraySizes, 0, sizeof( genShader.uniformsArraySizes ) );

	genShader.uniformsArraySizes[UNIFORM_DIFFUSEMAP] = 1;
	genShader.samplersArraySizes[TB_DIFFUSEMAP] = GetTextureCount( samplerMapping, TB_DIFFUSEMAP, shader->numUnfoggedPasses );
	
	if ( permutation & GENERICDEF_USE_LIGHTMAP )
	{
		genShader.uniformsArraySizes[UNIFORM_LIGHTMAP] = 1;
		genShader.samplersArraySizes[TB_LIGHTMAP] = GetTextureCount( samplerMapping, TB_LIGHTMAP, shader->numUnfoggedPasses );;
	}

	#if 0
	if ( permutation & GENERICDEF_USE_NORMALMAP )
	{
		// Per unique normal map
		genShader.uniformsArraySizes[UNIFORM_NORMALMAP] = 1;
		genShader.samplersArraySizes[TB_NORMALMAP] = 1;
	}

	if ( permutation & GENERICDEF_USE_DELUXEMAP )
	{
		// Per unique deluxe map
		genShader.uniformsArraySizes[UNIFORM_DELUXEMAP] = 1;
		genShader.samplersArraySizes[TB_DELUXEMAP] = 1;
	}

	if ( permutation & GENERICDEF_USE_SPECULARMAP )
	{
		// Per unique deluxe map
		genShader.uniformsArraySizes[UNIFORM_SPECULARMAP] = 1;
		genShader.samplersArraySizes[TB_SPECULARMAP] = 1;
	}

	if ( permutation & GENERICDEF_USE_SHADOWMAP )
	{
		genShader.uniformsArraySizes[UNIFORM_SHADOWMAP] = 1;
		genShader.samplersArraySizes[TB_SHADOWMAP] = 1;
	}

	if ( permutation & GENERICDEF_USE_CUBEMAP )
	{
		genShader.uniformsArraySizes[UNIFORM_CUBEMAP] = 1;
		genShader.samplersArraySizes[TB_CUBEMAP] = 1;
	}

	if ( permutation & (GENERICDEF_USE_NORMALMAP | GENERICDEF_USE_SPECULARMAP | GENERICDEF_USE_CUBEMAP) )
	{
		genShader.uniformsArraySizes[UNIFORM_ENABLETEXTURES] = 1;
	}

	if ( permutation & (GENERICDEF_USE_LIGHTVECTOR | GENERICDEF_USE_FASTLIGHT) )
	{
		genShader.uniformsArraySizes[UNIFORM_DIRECTLIGHT] = 1;
		genShader.uniformsArraySizes[UNIFORM_AMBIENTLIGHT] = 1;
	}

	if ( permutation & (GENERICDEF_USE_PRIMARYLIGHT | GENERICDEF_USE_SHADOWMAP) )
	{
		genShader.uniformsArraySizes[UNIFORM_PRIMARYLIGHTCOLOR] = 1;
		genShader.uniformsArraySizes[UNIFORM_PRIMARYLIGHTAMBIENT] = 1;
	}

	if ( (permutation & GENERICDEF_USE_LIGHT) != 0 &&
			(permutation & GENERICDEF_USE_FASTLIGHT) == 0 )
	{
		genShader.uniformsArraySizes[UNIFORM_NORMALSCALE] = 1;
		genShader.uniformsArraySizes[UNIFORM_SPECULARSCALE] = 1;
	}
	u_
	if ( (permutation & GENERICDEF_USE_LIGHT) != 0 &&
			(permutation & GENERICDEF_USE_FASTLIGHT) == 0 &&
			(permutation & GENERICDEF_USE_CUBEMAP) != 0 )
	{
		genShader.uniformsArraySizes[UNIFORM_CUBEMAP] = 1;
		genShader.samplersArraySizes[TB_CUBEMAP] = 1;
	}
	#endif
	
	// Generate the code
	AddShaderHeaderCode( shader, defines, permutation, code );

	code << '\n';

	// Add uniforms
	for ( int i = 0; i < UNIFORM_COUNT; i++ )
	{
		if ( genShader.uniformsArraySizes[i] > 0 )
		{
			code << "uniform " << glslTypeStrings[uniformsInfo[i].type] << ' ' << uniformsInfo[i].name;
			code << '[' << (uniformsInfo[i].size * genShader.uniformsArraySizes[i]) << ']';

			code << ";\n";
		}
	}

	code << '\n';

	// 
	// Add inputs
	//
	if ( shader->numUnfoggedPasses > 0 )
	{
		code << "in vec2 var_TexCoords[" << shader->numUnfoggedPasses << "];\n";

		if ( permutation & GENERICDEF_USE_LIGHTMAP )
		{
			code << "in vec2 var_LightTex;\n";
		}

		if ( permutation & GENERICDEF_USE_RGBAGEN )
		{
			code << "in vec4 var_Colors[" << shader->numUnfoggedPasses << "];\n";
		}

		code << '\n';
	}

	//
	// Add outputs
	//
	code << "out vec4 out_Color;\n";
	code << "out vec4 out_Glow;\n";

	code << '\n';

	//
	// Main function body
	//
	code << "void main() {\n\
	\n\
	vec4 src = vec4(0.0);\n\
	vec4 src2 = vec4(0.0);\n\
	vec4 dst = vec4(0.0);\n\n";

	for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
	{
		const shaderStage_t *stage = shader->stages[i];

		code << "	// Stage " << (i + 1) << '\n';

		if ( stage->bundle[TB_DIFFUSEMAP].image[0] != NULL )
		{
			code << "	src = texture(u_DiffuseMap[" << samplerMapping[i][TB_DIFFUSEMAP] << "], var_TexCoords[" << i << "]);\n";

			if ( stage->bundle[TB_LIGHTMAP].image[0] != NULL )
			{
				code << "	src2 = texture(u_LightMap[" << samplerMapping[i][TB_LIGHTMAP] << "],  var_LightTex);\n";
				code << "	src.rgb = src.rgb * src2.rgb;\n";
			}
		}

		switch ( stage->rgbGen )
		{
			case CGEN_CONST:
				code << "	src.rgb *= vec3("
						<< (stage->constantColor[0] / 255.0f) << ", "
						<< (stage->constantColor[1] / 255.0f) << ", "
						<< (stage->constantColor[2] / 255.0f)
						<< ");\n";
				break;

			default:
				break;
		}
		
		switch ( stage->alphaGen )
		{
			case AGEN_CONST:
				code << "	src.a *= " << (stage->constantColor[3] / 255.0f) << ";\n";
				break;

			default:
				break;
		}
		
		// Depends on blend function
		AddBlendEquationCode(code, stage->stateBits);

		code << '\n';
	}

	//if ( permutation & GENERICDEF_USE_RGBAGEN )
	{
		code << "	out_Color = dst;\n";
	}
	//else
	//{
	//	code << "	out_Color = color;\n";
	//}

	if ( permutation & GENERICDEF_USE_GLOW_BUFFER )
	{
		code << "	out_Glow = out_Color;\n";
	}
	else
	{
		code << "	out_Glow = vec4(0.0);\n";
	}

	code << '}';
}

uint32_t GetShaderCapabilities( const shader_t *shader )
{
	uint32_t caps = 0;

	if ( shader->numDeforms )
	{
		caps |= GENERICDEF_USE_DEFORM_VERTEXES;
	}

	if ( shader->numUnfoggedPasses == 0 )
	{
		caps |= GENERICDEF_USE_FOG;
	}

	for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
	{
		const shaderStage_t *stage = shader->stages[i];

		if ( stage->bundle[TB_LIGHTMAP].image[0] != NULL )
		{
			caps |= GENERICDEF_USE_LIGHTMAP;
		}

		if ( stage->rgbGen == CGEN_LIGHTING_DIFFUSE ||
				stage->alphaGen == AGEN_LIGHTING_SPECULAR ||
				stage->alphaGen == AGEN_PORTAL )
		{
			caps |= GENERICDEF_USE_RGBAGEN;
		}

		for ( int j = 0; stage->bundle[j].image[0] != NULL; j++ )
		{
			if ( stage->bundle[j].tcGen != TCGEN_TEXTURE ||
					stage->bundle[j].numTexMods > 0 )
			{
				caps |= GENERICDEF_USE_TCGEN_AND_TCMOD;
			}
		}

		if ( stage->glow )
		{
			caps |= GENERICDEF_USE_GLOW_BUFFER;
		}
	}

	return caps;
}

static size_t GetSizeOfGLSLTypeInBytes( int type )
{
	switch ( type )
	{
		case GLSL_INT: // fallthrough
		case GLSL_SAMPLER2D: // fallthrough
		case GLSL_SAMPLERCUBE: return sizeof( int );
		case GLSL_FLOAT: return sizeof( float );
		case GLSL_VEC2: return sizeof( float ) * 2;
		case GLSL_VEC3: return sizeof( float ) * 3;
		case GLSL_VEC4: return sizeof( float ) * 4;
		case GLSL_MAT16: return sizeof( float ) * 16;
		default: assert( !"Invalid GLSL type" ); return 0;
	}
}

void SetMaterialData( Material *material, void *newData, uniform_t uniform, int offset, int count )
{
	ShaderProgram *program = material->program;
	ConstantDescriptor *constantData = &program->constants[program->uniformDataIndices[uniform]];
	char *data = reinterpret_cast<char *>(material->constantData) + constantData->offset;
	size_t baseTypeSizeInBytes = GetSizeOfGLSLTypeInBytes( constantData->type );

	assert( count > 0 );

	memcpy( data + baseTypeSizeInBytes * offset, newData,
		baseTypeSizeInBytes * constantData->numElementsInBaseType * count );
}

static void FillDefaultUniformData( Material *material, const shader_t *shader )
{
	ShaderProgram *program = material->program;
	for ( int i = 0,  end = program->numUniforms; i < end; i++ )
	{
		const ConstantDescriptor *uniform = &program->constants[i];
		void *data = reinterpret_cast<char *>(material->constantData) + uniform->offset;

		switch ( uniform->uniform )
		{
			case UNIFORM_DIFFUSEMAP:
				*(int *)data = TB_DIFFUSEMAP;
				break;

			case UNIFORM_LIGHTMAP:
				*(int *)data = TB_LIGHTMAP;
				break;

			case UNIFORM_NORMALMAP:
				*(int *)data = TB_NORMALMAP;
				break;

			case UNIFORM_DELUXEMAP:
				*(int *)data = TB_DELUXEMAP;
				break;

			case UNIFORM_SPECULARMAP:
				*(int *)data = TB_SPECULARMAP;
				break;

			case UNIFORM_TEXTUREMAP:
				*(int *)data = TB_COLORMAP;
				break;

			case UNIFORM_LEVELSMAP:
				*(int *)data = TB_LEVELSMAP;
				break;

			case UNIFORM_CUBEMAP:
				*(int *)data = TB_CUBEMAP;
				break;

			case UNIFORM_SCREENIMAGEMAP:
				*(int *)data = TB_COLORMAP;
				break;

			case UNIFORM_SCREENDEPTHMAP:
				*(int *)data = TB_COLORMAP;
				break;

			case UNIFORM_SHADOWMAP:
				*(int *)data = TB_SHADOWMAP;
				break;

			case UNIFORM_SHADOWMAP2:
				*(int *)data = TB_SHADOWMAP2;
				break;

			case UNIFORM_SHADOWMAP3:
				*(int *)data = TB_SHADOWMAP3;
				break;

			case UNIFORM_SHADOWMVP:
			case UNIFORM_SHADOWMVP2:
			case UNIFORM_SHADOWMVP3:
				// Dynamic data, no need to initialize
				break;

			case UNIFORM_ENABLETEXTURES:
				VectorSet4( (float *)data, 0.0f, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_DIFFUSETEXMATRIX:
				VectorSet4( (float *)data, 0.0f, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_DIFFUSETEXOFFTURB:
				VectorSet4( (float *)data, 0.0f, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_TEXTURE1ENV:
				if ( shader->stages[0]->bundle[TB_LIGHTMAP].image[0] )
				{
					if ( r_lightmap->integer )
					{
						*(int *)data = GL_REPLACE;
					}
					else
					{
						*(int *)shader->multitextureEnv;
					}
				}
				else
				{
					*(int *)data = 0;
				}
				break;

			case UNIFORM_TCGEN0:
				*(int *)data = shader->stages[0]->bundle[0].tcGen;
				break;

			case UNIFORM_TCGEN0VECTOR0:
				VectorSet( (float *)data, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_TCGEN0VECTOR1:
				VectorSet( (float *)data, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_DEFORMFUNC:
				*(int *)data = GF_NONE;
				break;

			case UNIFORM_DEFORMTYPE:
				*(int *)data = DEFORM_NONE;
				break;

			case UNIFORM_DEFORMPARAMS:
				((float *)data)[0] = 0.0f;
				((float *)data)[1] = 0.0f;
				((float *)data)[2] = 0.0f;
				((float *)data)[3] = 0.0f;
				((float *)data)[4] = 0.0f;
				((float *)data)[5] = 0.0f;
				((float *)data)[6] = 0.0f;
				break;

			case UNIFORM_COLORGEN:
				*(int *)data = CGEN_LIGHTING_DIFFUSE_ENTITY;
				break;

			case UNIFORM_ALPHAGEN:
				*(int *)data = AGEN_IDENTITY;
				break;

			case UNIFORM_COLOR:
				VectorSet4( (float *)data, 0.0f, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_BASECOLOR:
				VectorSet4( (float *)data, 0.0f, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_VERTCOLOR:
				VectorSet4( (float *)data, 0.0f, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_DLIGHTINFO:
				VectorSet4( (float *)data, 0.0f, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_LIGHTFORWARD:
				VectorSet( (float *)data, 0.0f, 1.0f, 0.0f );
				break;

			case UNIFORM_LIGHTUP:
				VectorSet( (float *)data, 0.0f, 0.0f, 1.0f );
				break;

			case UNIFORM_LIGHTRIGHT:
				VectorSet( (float *)data, 1.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_LIGHTORIGIN:
				VectorSet4( (float *)data, 0.0f, 0.0f, 0.0f, 1.0f );
				break;

			case UNIFORM_MODELLIGHTDIR:
				VectorSet( (float *)data, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_LIGHTRADIUS:
				*(float *)data = 100.0f;
				break;

			case UNIFORM_AMBIENTLIGHT:
				VectorSet( (float *)data, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_DIRECTEDLIGHT:
				VectorSet( (float *)data, 0.0f, 0.0f, 0.0f );
				break;

			case UNIFORM_PORTALRANGE:
				*(float *)data = shader->portalRange;
				break;

			case UNIFORM_FOGDISTANCE:
				*(int *)data = 1;
				break;

			case UNIFORM_FOGDEPTH:
				*(int *)data = 1;
				break;

			case UNIFORM_FOGEYET:
				*(int *)data = 1;
				break;

			case UNIFORM_FOGCOLORMASK:
				*(int *)data = 1;
				break;

			case UNIFORM_MODELMATRIX:
				*(int *)data = 1;
				break;

			case UNIFORM_MODELVIEWPROJECTIONMATRIX:
				*(int *)data = 1;
				break;

			case UNIFORM_TIME:
				*(int *)data = 1;
				break;

			case UNIFORM_VERTEXLERP:
				*(int *)data = 1;
				break;

			case UNIFORM_NORMALSCALE:
				VectorCopy4( (float *)data, shader->stages[0]->normalScale );
				break;

			case UNIFORM_SPECULARSCALE:
				VectorCopy4( (float *)data, shader->stages[0]->specularScale );
				break;

			case UNIFORM_VIEWINFO: // znear: zfar: width/2: height/2
				*(int *)data = 1;
				break;

			case UNIFORM_VIEWORIGIN:
				*(int *)data = 1;
				break;

			case UNIFORM_LOCALVIEWORIGIN:
				*(int *)data = 1;
				break;

			case UNIFORM_VIEWFORWARD:
				*(int *)data = 1;
				break;

			case UNIFORM_VIEWLEFT:
				*(int *)data = 1;
				break;

			case UNIFORM_VIEWUP:
				*(int *)data = 1;
				break;

			case UNIFORM_INVTEXRES:
				*(int *)data = 1;
				break;

			case UNIFORM_AUTOEXPOSUREMINMAX:
				*(int *)data = 1;
				break;

			case UNIFORM_TONEMINAVGMAXLINEAR:
				*(int *)data = 1;
				break;

			case UNIFORM_PRIMARYLIGHTORIGIN:
				*(int *)data = 1;
				break;

			case UNIFORM_PRIMARYLIGHTCOLOR:
				*(int *)data = 1;
				break;

			case UNIFORM_PRIMARYLIGHTAMBIENT:
				*(int *)data = 1;
				break;

			case UNIFORM_PRIMARYLIGHTRADIUS:
				*(int *)data = 1;
				break;

			case UNIFORM_CUBEMAPINFO:
				*(int *)data = 1;
				break;

			case UNIFORM_BONE_MATRICES:
				*(int *)data = 1;
				break;

			default:
				assert(!"Invalid uniform enum");
				break;
		}
	}
}

static void GenerateSamplingTable( const shader_t *shader, int samplerMapping[MAX_SHADER_STAGES][NUM_TEXTURE_BUNDLES] )
{
	for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
	{
		const shaderStage_t *stage = shader->stages[i];
		int numTextures[NUM_TEXTURE_BUNDLES] = {};

		for ( int j = 0; j < NUM_TEXTURE_BUNDLES; j++ )
		{
			if ( stage->bundle[j].image[0] != NULL )
			{
				samplerMapping[i][j] = numTextures[j]++;
			}
			else
			{
				samplerMapping[i][j] = -1;
			}
		}
	}
}

static void AssignSamplersAndTextures( Material *material, const shader_t *shader )
{
	ShaderProgram *shaderProgram = material->program;
	int totalNumSamplers = 0;

	if ( shaderProgram->numSamplers == 0 )
	{
		return;
	}

	for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
	{
		const shaderStage_t *stage = shader->stages[i];
		GLuint samplers[NUM_TEXTURE_BUNDLES];
		int numSamplers = 0;

		for ( int j = 0; j < NUM_TEXTURE_BUNDLES; j++ )
		{
			if ( shaderProgram->samplerIndexes[i][j] != -1 )
			{
				samplers[numSamplers] = totalNumSamplers;
				material->textures[totalNumSamplers] = stage->bundle[j].image[0]->texnum;

				numSamplers++;
				totalNumSamplers++;
			}

			SetMaterialData( material, samplers, bundleToUniform[j], 0, numSamplers );
		}
	}
}

// Slight variation on the MurmurHash - we know that the size of data is a multiple of 4.
static uint32_t MurmurHash( uint32_t *key, uint32_t numDWords, uint32_t seed )
{
	static const uint32_t c1 = 0xcc9e2d51;
	static const uint32_t c2 = 0x1b873593;
	static const uint32_t r1 = 15;
	static const uint32_t r2 = 13;
	static const uint32_t m = 5;
	static const uint32_t n = 0xe6546b64;

	uint32_t hash = seed;
	for ( uint32_t i = 0; i < numDWords; i++ )
	{
		uint32_t k = key[i];

		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;

		hash ^= k;
		hash = (hash << r2) | (hash >> (32 - r2));
		hash *= m + n;
	}

	hash ^= numDWords * 4;
	hash ^= hash >> 16;
	hash *= 0x85ebca6b;
	hash ^= hash >> 13;
	hash *= 0xc2b2ae35;
	hash ^= hash >> 16;

	return hash;
}

static uint32_t CalculateShaderHash( const shader_t *shader, uint32_t permutation )
{
	struct CompactShader
	{
		uint32_t permutation;

		unsigned numDeforms : 2;
		unsigned numStages : 3;
		unsigned glow : 1;

		struct
		{
			struct
			{
				unsigned tcGenType : 4;
				unsigned numTexMods : 2;
				unsigned numAnimImages : 5;
			} bundle[NUM_TEXTURE_BUNDLES];

			unsigned rgbWaveType : 3;
			unsigned alphaWaveType : 3;
			unsigned fogColorAdjust : 2;
			uint32_t stateBits;

			byte constColor[4];
		} stages[MAX_SHADER_STAGES];
	};

	byte data[(sizeof( CompactShader ) * 4 + 3) / 4]; // Rounded up to nearest 4 bytes
	CompactShader *compact = reinterpret_cast<CompactShader *>(data);

	memset( data, 0, sizeof( data ) );

	compact->permutation = permutation;
	compact->numDeforms = shader->numDeforms;
	compact->numStages = shader->numUnfoggedPasses;

	for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
	{
		const shaderStage_t *stage = shader->stages[i];

		compact->glow = stage->glow || compact->glow;

		for ( int j = 0; stage->bundle[j].image[0] != NULL; j++ )
		{
			compact->stages[i].bundle[j].numAnimImages = stage->bundle[j].numImageAnimations;
			compact->stages[i].bundle[j].numTexMods = stage->bundle[j].numTexMods;
			compact->stages[i].bundle[j].tcGenType = stage->bundle[j].tcGen;
		}
		
		compact->stages[i].rgbWaveType = stage->rgbWave.func;
		compact->stages[i].alphaWaveType = stage->alphaWave.func;
		compact->stages[i].fogColorAdjust = stage->adjustColorsForFog;
		compact->stages[i].stateBits = stage->stateBits;
		memcpy( compact->stages[i].constColor, stage->constantColor, sizeof( stage->constantColor ) );
	}

	uint32_t *hashData = reinterpret_cast<uint32_t *>(data);
	size_t numDWords = sizeof( data ) / 4;
	uint32_t hash = MurmurHash( hashData, numDWords, 53271 );

	return hash & (GLSLGeneratorContext::MAX_SHADER_PROGRAM_TABLE_SIZE - 1);
}

Material *GLSLGeneratorGenerateMaterial( GLSLGeneratorContext *ctx, const shader_t *shader, const char *defines[], uint32_t permutation )
{
	// Calculate hash for shader

	permutation |= GetShaderCapabilities( shader );

	uint32_t shaderHash = CalculateShaderHash( shader, permutation );
	ShaderProgram *shaderProgram = ctx->shaderProgramsTable[shaderHash];

	while ( shaderProgram != NULL )
	{
		if ( shaderProgram->permutation == permutation )
		{
			break;
		}

		shaderProgram = shaderProgram->next;
	}

	if ( shaderProgram == NULL )
	{
		GLSLShader vertexShader;
		GLSLShader fragmentShader;

		memset( vertexShader.textures, 0, sizeof( vertexShader.textures ) );
		memset( fragmentShader.textures, 0, sizeof( fragmentShader.textures ) );

		int uniformArraySizes[UNIFORM_COUNT];
		int uniformIndexes[UNIFORM_COUNT];
		int numUniforms;
		int samplerMapping[MAX_SHADER_STAGES][NUM_TEXTURE_BUNDLES];
		int numSamplers;

		// Generate shader code
		GenerateSamplingTable( shader, samplerMapping );

		GenerateGenericVertexShaderCode(
			shader,
			defines,
			permutation,
			samplerMapping,
			vertexShader );

		GenerateGenericFragmentShaderCode(
			shader,
			defines,
			permutation,
			samplerMapping,
			fragmentShader );

		numUniforms = 0;
		for ( int i = 0; i < UNIFORM_COUNT; i++ )
		{
			uniformArraySizes[i] = max(
				vertexShader.uniformsArraySizes[i],
				fragmentShader.uniformsArraySizes[i]);

			if ( uniformArraySizes[i] > 0 )
			{
				uniformIndexes[numUniforms++] = i;
			}
		}

		numSamplers = 0;
		for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
		{
			for ( int j = 0; shader->stages[i]->bundle[j].image[0] != NULL; j++ )
			{
				const textureBundle_t *bundle = &shader->stages[i]->bundle[j];

				for ( int k = 0; bundle->image[k] != NULL; k++ )
				{
					numSamplers++;
				}
			}
		}

		// Generate static data and allocate memory for dynamic data

		// FIXME: Refactor into tr_glsl.cpp
		std::string vshaderString = vertexShader.code.str();
		std::string fshaderString = fragmentShader.code.str();

		const char *vertexShaderCode = vshaderString.c_str();
		const char *fragmentShaderCode = fshaderString.c_str();

		int vshader = qglCreateShader( GL_VERTEX_SHADER );
		if ( vshader == 0 )
		{
			// Error etc?
			return NULL;
		}

		qglShaderSource( vshader, 1, &vertexShaderCode, NULL );
		qglCompileShader( vshader );

		int fshader = qglCreateShader( GL_FRAGMENT_SHADER );
		if ( fshader == 0 )
		{
			// Error etc
			return NULL;
		}

		qglShaderSource( fshader, 1, &fragmentShaderCode, NULL );
		qglCompileShader( fshader );

		int program = qglCreateProgram();
		if ( program == 0 )
		{
			// Error etc
			return NULL;
		}

		qglAttachShader( program, vshader );
		qglAttachShader( program, fshader );

		for ( int i = 0; i < ATTR_INDEX_COUNT; i++ )
		{
			if ( vertexShader.attributes & (1 << i) )
			{
				qglBindAttribLocation( program, i, attributeMeta[i].name );
			}
		}

		qglLinkProgram( program );

		// FIXME: Lol, leak memory
		shaderProgram = new ShaderProgram();
		shaderProgram->used = qfalse;
		shaderProgram->next = ctx->shaderProgramsTable[shaderHash];
		shaderProgram->permutation = permutation;
		shaderProgram->program = program;
		shaderProgram->fragmentShader = fshader;
		shaderProgram->vertexShader = vshader;
		shaderProgram->numUniforms = numUniforms;
		shaderProgram->constants = new ConstantDescriptor[numUniforms];
		memcpy( shaderProgram->samplerIndexes, samplerMapping, sizeof( shaderProgram->samplerIndexes ) );
		shaderProgram->numSamplers = numSamplers;
		shaderProgram->samplers = new SamplerDescriptor[numSamplers];

		int constantBufferSizeInBytes = 0;
		for ( int i = 0; i < numUniforms; i++ )
		{
			ConstantDescriptor *uniform = &shaderProgram->constants[i];
			int uniformIndex = uniformIndexes[i];
			const uniformInfo_t *uniformInfo = &uniformsInfo[uniformIndex];

			uniform->uniform = uniformIndex;
			uniform->offset = constantBufferSizeInBytes;
			uniform->numElementsInBaseType = uniformInfo->size;
			uniform->totalNumElements = uniformInfo->size * uniformArraySizes[uniformIndex];
			uniform->type = uniformInfo->type;
			uniform->location = -1;

			shaderProgram->uniformDataIndices[uniformIndex] = i;

			constantBufferSizeInBytes += uniform->totalNumElements * GetSizeOfGLSLTypeInBytes( uniform->type );
		}

		shaderProgram->constantsBufferSizeInBytes = constantBufferSizeInBytes;

		int samplerIndex = 0;
		for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
		{
			for ( int j = 0; shader->stages[i]->bundle[j].image[0] != NULL; j++ )
			{
				SamplerDescriptor *sampler = &shaderProgram->samplers[samplerIndex];

				switch ( j )
				{
					case TB_CUBEMAP:
						sampler->target = GL_TEXTURE_CUBE_MAP;
						samplerIndex++;
						break;

					default:
						sampler->target = GL_TEXTURE_2D;
						samplerIndex++;
						break;
				}
			}
		}

		// And insert it into the hash table
		ctx->shaderProgramsTable[shaderHash] = shaderProgram;

		numGLSLPrograms++;
	} // if hash not exists

	Material *material = new Material();

	material->program = shaderProgram;
	material->constantData = new char[shaderProgram->constantsBufferSizeInBytes];
	material->textures = new GLuint[shaderProgram->numSamplers];
	
	AssignSamplersAndTextures( material, shader );
	FillDefaultUniformData( material, shader );

	numQ3ShadersLoaded++;
	
	Com_Printf("Q3 shaders loaded: %d\n", numQ3ShadersLoaded);
	Com_Printf("GLSL programs generated: %d\n", numGLSLPrograms);
	
	return material;
}

void GLSLGeneratorFreeMaterial( Material *material )
{
	delete[] material->constantData;
	delete[] material->textures;
	delete material;
}

void RB_MakeMaterialReady( Material *material )
{
	ShaderProgram *program = material->program;

	if ( program->used )
	{
		return;
	}
	
	std::string log;
	GLint status;
	qglGetShaderiv( program->vertexShader, GL_COMPILE_STATUS, &status );
	if ( status != GL_TRUE )
	{
		GLint logLength;
		qglGetShaderiv( program->vertexShader, GL_INFO_LOG_LENGTH, &logLength );

		log.resize( logLength );
		
		qglGetShaderInfoLog( program->vertexShader, logLength, NULL, &log[0] );
		Com_Printf( "Vertex shader error: %s\n", log.c_str() );
	}

	qglGetShaderiv( program->fragmentShader, GL_COMPILE_STATUS, &status );
	if ( status != GL_TRUE )
	{
		GLint logLength;
		qglGetShaderiv( program->fragmentShader, GL_INFO_LOG_LENGTH, &logLength );

		log.resize( logLength );
		
		qglGetShaderInfoLog( program->fragmentShader, logLength, NULL, &log[0] );
		Com_Printf( "Fragment shader error: %s\n", log.c_str() );
	}

	qglGetProgramiv( program->program, GL_LINK_STATUS, &status );
	if ( status != GL_TRUE )
	{
		GLint logLength;
		qglGetProgramiv( program->program, GL_INFO_LOG_LENGTH, &logLength );

		log.resize( logLength );
		
		qglGetProgramInfoLog( program->program, logLength, NULL, &log[0] );
		Com_Printf( "Linker error: %s\n", log.c_str() );
	}

	qglUseProgram( program->program );

	for ( int i = 0; i < program->numUniforms; i++ )
	{
		const uniformInfo_t *uniform = &uniformsInfo[program->constants[i].uniform];
		program->constants[i].location = qglGetUniformLocation( program->program, uniform->name );
	}

	program->used = true;
}

void RB_BindMaterial( const Material *material )
{
	const ShaderProgram *program = material->program;

	// Sneaky...
	RB_MakeMaterialReady( const_cast<Material *>(material) );

	qglUseProgram( program->program );

	// Update uniforms
	for ( int i = 0, end = program->numUniforms; i < end; i++ )
	{
		const ConstantDescriptor *constantData = &program->constants[i];
		const GLfloat *data = reinterpret_cast<const GLfloat *>(material->constantData) + constantData->offset;

		switch ( constantData->type )
		{
			case GLSL_INT:
			case GLSL_SAMPLER2D:
			case GLSL_SAMPLERCUBE:
				qglUniform1iv( constantData->location, constantData->totalNumElements, reinterpret_cast<const GLint *>(data) );
				break;

			case GLSL_FLOAT:
				qglUniform1fv( constantData->location, constantData->totalNumElements, data );
				break;

			case GLSL_VEC2:
				qglUniform2fv( constantData->location, constantData->totalNumElements, data );
				break;

			case GLSL_VEC3:
				qglUniform3fv( constantData->location, constantData->totalNumElements, data );
				break;

			case GLSL_VEC4:
				qglUniform4fv( constantData->location, constantData->totalNumElements, data );
				break;

			case GLSL_MAT16:
				qglUniformMatrix4fv( constantData->location, constantData->totalNumElements, GL_FALSE, data );
				break;

			default:
				assert( false );
				break;
		}
	}

	// Bind textures
	for ( int i = 0; i < program->numSamplers; i++ )
	{
		GL_BindTexture( program->samplers[i].target, material->textures[i], i );
	}
}