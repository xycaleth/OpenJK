#include "tr_local.h"

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
static const char *attributeStrings[] = {
	"vec3 attr_Position;\n",
	"vec2 attr_TexCoord0;\n",
	"vec2 attr_TexCoord1;\n",
	"vec4 attr_Tangent;\n",
	"vec3 attr_Normal;\n",
	"vec4 attr_Color;\n",
	"vec4 attr_PaintColor;\n",
	"vec3 attr_LightDirection;\n",
	"vec4 attr_BoneIndexes;\n",
	"vec4 attr_BoneWeights;\n",
	"vec3 attr_Position2;\n",
	"vec3 attr_Tangent2;\n",
	"vec4 attr_Normal2;\n"
};

static const char *glslTypeStrings[] = {
	"int",
	"float",
	"float[5]",
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
render pass: {depth prepass, shadow, render, fog} <-- depth prepass and shader the same thing?
*/

const int UNIFORMS_BITFIELD_ARRAY_SIZE = (UNIFORM_COUNT + 31) / 32;
const int SAMPLERS_BITFIELD_ARRAY_SIZE = (NUM_TEXTURE_BUNDLES + 31) / 32;

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
	std::string code;

	uint32_t attributes;
	uint32_t samplers[SAMPLERS_BITFIELD_ARRAY_SIZE];
	uint32_t uniforms[UNIFORMS_BITFIELD_ARRAY_SIZE];

	int attributesArraySizes[ATTR_INDEX_COUNT];
	int samplersArraySizes[NUM_TEXTURE_BUNDLES];
	int uniformsArraySizes[UNIFORM_COUNT];
};

struct ConstantDescriptor
{
	int type;
	int offset;
	int location;
	int size;
};

struct SamplerDescriptor
{
	GLenum target;
};

struct ShaderProgram
{
	int vertexShader;
	int fragmentShader;
	int program;

	int numSamplers;
	SamplerDescriptor *samplers;

	int numUniforms;
	ConstantDescriptor *constants;

	// Mapping from UNIFORM_* -> array index into 'constants' array.
	int uniformDataIndices[UNIFORM_COUNT];
};

struct Material
{
	ShaderProgram *program;

	void *constantData;
	GLuint *textures;
};

const int MAX_SHADER_PROGRAM_TABLE_SIZE = 2048;
static ShaderProgram *shaderProgramsTable[MAX_SHADER_PROGRAM_TABLE_SIZE];

static uint32_t GenerateVertexShaderHashValue( const shader_t *shader, uint32_t permutation )
{
	return shader->index;
}

static uint32_t GenerateFragmentShaderHashValue( const shader_t *shader, uint32_t permutation )
{
	return shader->index;
}

static bool AddShaderHeaderCode( const shader_t *shader, const char *defines[], uint32_t permutation, std::string& code )
{
	// Add permutations for debug
	code += "// Generic shader\n";

	if ( permutation == 0 )
	{
		code += "// No shader flags\n";
	}
	else
	{
		for ( int i = 0, j = 1; i < ARRAY_LEN(genericShaderDefStrings); i++, j <<= 1 )
		{
			if ( permutation & j )
			{
				code += "// ";
				code += genericShaderDefStrings[i];
				code += '\n';
			}
		}
	}

	if ( defines != NULL )
	{
		for ( const char **define = defines; *define != NULL; define += 2 )
		{
			code += "#define ";
			code += define[0];
			code += ' ';
			code += define[1];
			code += '\n';
		}
	}

	code += "#line 0\n";
	code += "#version 150 core\n";

	return true;
}

static void GenerateGenericVertexShaderCode(
	const shader_t *shader,
	const char *defines[],
	uint32_t permutation,
	GLSLShader& genShader )
{
	std::string& code = genShader.code;
	uint32_t attributes = shader->vertexAttribs;

	// Determine inputs needed
	memset( genShader.attributesArraySizes, 1, sizeof( genShader.attributesArraySizes ) );
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

	if ( permutation & (GENERICDEF_USE_TCGEN_AND_TCMOD | GENERICDEF_USE_RGBAGEN) )
	{
		attributes |= ATTR_TEXCOORD1;
	}

	// Determine uniforms needed
	memset( genShader.uniforms, 0, sizeof( genShader.uniforms ) );
	memset( genShader.uniformsArraySizes, 1, sizeof( genShader.uniformsArraySizes ) );
	memset( genShader.samplersArraySizes, 1, sizeof( genShader.samplersArraySizes ) );

	SetBit( genShader.uniforms, UNIFORM_MODELVIEWPROJECTIONMATRIX );
	SetBit( genShader.uniforms, UNIFORM_BASECOLOR );
	SetBit( genShader.uniforms, UNIFORM_VERTCOLOR );
	
	if ( permutation & (GENERICDEF_USE_TCGEN_AND_TCMOD | GENERICDEF_USE_RGBAGEN) )
	{
		SetBit( genShader.uniforms, UNIFORM_LOCALVIEWORIGIN );
	}

	if ( permutation & GENERICDEF_USE_TCGEN_AND_TCMOD )
	{
		// Per unique diffuse tex mods
		SetBit( genShader.uniforms, UNIFORM_DIFFUSETEXMATRIX );
		SetBit( genShader.uniforms, UNIFORM_DIFFUSETEXOFFTURB );

		// Per unique tc gens
		SetBit( genShader.uniforms, UNIFORM_TCGEN0 );
		SetBit( genShader.uniforms, UNIFORM_TCGEN0VECTOR0 );
		SetBit( genShader.uniforms, UNIFORM_TCGEN0VECTOR1 );
	}

	if ( permutation & GENERICDEF_USE_FOG )
	{
		SetBit( genShader.uniforms, UNIFORM_FOGCOLORMASK );
		SetBit( genShader.uniforms, UNIFORM_FOGDEPTH );
		SetBit( genShader.uniforms, UNIFORM_FOGDISTANCE );
		SetBit( genShader.uniforms, UNIFORM_FOGEYET );
	}

	if ( permutation & GENERICDEF_USE_DEFORM_VERTEXES )
	{
		// Per deform vertex entry
		SetBit( genShader.uniforms, UNIFORM_DEFORMGEN );
		SetBit( genShader.uniforms, UNIFORM_DEFORMPARAMS );
		SetBit( genShader.uniforms, UNIFORM_TIME );

		genShader.uniformsArraySizes[UNIFORM_DEFORMGEN] = shader->numDeforms;
		genShader.uniformsArraySizes[UNIFORM_DEFORMPARAMS] = shader->numDeforms;
	}

	if ( permutation & GENERICDEF_USE_RGBAGEN )
	{
		// Per stage?
		SetBit( genShader.uniforms, UNIFORM_COLORGEN );
		SetBit( genShader.uniforms, UNIFORM_ALPHAGEN );
		SetBit( genShader.uniforms, UNIFORM_AMBIENTLIGHT );
		SetBit( genShader.uniforms, UNIFORM_DIRECTEDLIGHT );
		SetBit( genShader.uniforms, UNIFORM_MODELLIGHTDIR );
		SetBit( genShader.uniforms, UNIFORM_PORTALRANGE );
	}

	// Generate the code
	AddShaderHeaderCode( shader, defines, permutation, code );

	code += '\n';

	// Add uniforms
	for ( int i = 0; i < UNIFORM_COUNT; i++ )
	{
		if ( IsBitSet( genShader.uniforms, i ) )
		{
			code += "uniform ";
			code += glslTypeStrings[uniformsInfo[i].type];

			if ( uniformsInfo[i].size > 1 )
			{
				// TODO: Add array specifier
			}

			code += ' ';
			code += uniformsInfo[i].name;
			code += ";\n";
		}
	}

	code += '\n';

	//
	// Add inputs
	//
	assert( sizeof( int ) == 4 );
	for ( int i = 0, j = 1; i < 32; i++, j <<= 1 )
	{
		if ( shader->vertexAttribs & j )
		{
			code += "in ";
			code += attributeStrings[i];
		}
	}

	code += '\n';

	// 
	// Add outputs
	//
	code += "out vec2 var_TexCoords[";
	code += (shader->numUnfoggedPasses + '0');
	code += "];\n";
	code += "out vec4 var_Color;\n";

	if ( permutation & GENERICDEF_USE_LIGHTMAP )
	{
		code += "out vec2 var_LightTex;\n";
	}

	code += '\n';

	// Helper functions
	if ( permutation & GENERICDEF_USE_TCGEN_AND_TCMOD )
	{
		code += "vec2 GenTexCoords(in int TCGen, in vec3 position, in vec3 normal, in vec3 TCGenVector0, in vec3 TCGenVector1)\n\
{\n\
	vec2 tex = attr_TexCoord0;\n\
	\n\
	if (TCGen >= TCGEN_LIGHTMAP && TCGen <= TCGEN_LIGHTMAP3)\n\
	{\n\
		tex = attr_TexCoord1;\n\
	}\n\
	else if (TCGen == TCGEN_ENVIRONMENT_MAPPED)\n\
	{\n\
		vec3 viewer = normalize(u_LocalViewOrigin - position);\n\
		vec2 ref = reflect(viewer, normal).yz;\n\
		tex.s = ref.x * -0.5 + 0.5;\n\
		tex.t = ref.y *  0.5 + 0.5;\n\
	}\n\
	else if (TCGen == TCGEN_VECTOR)\n\
	{\n\
		tex = vec2(dot(position, TCGenVector0), dot(position, TCGenVector1));\n\
	}\n\
	\n\
	return tex;\n\
}\n\n";

		code += "vec2 ModTexCoords(in vec2 st, in vec3 position, in vec4 texMatrix, in vec4 offTurb)\n\
{\n\
	float amplitude = offTurb.z;\n\
	float phase = offTurb.w * 2.0 * M_PI;\n\
	vec2 st2;\n\
	st2.x = st.x * texMatrix.x + (st.y * texMatrix.z + offTurb.x);\n\
	st2.y = st.x * texMatrix.y + (st.y * texMatrix.w + offTurb.y);\n\
	\n\
	vec2 offsetPos = vec2(position.x + position.z, position.y);\n\
	\n\
	vec2 texOffset = sin(offsetPos * (2.0 * M_PI / 1024.0) + vec2(phase));\n\
	\n\
	return st2 + texOffset * amplitude;\n\
}\n\n";
	}

	if ( permutation & GENERICDEF_USE_FOG )
	{
		code += "float CalcFog(in vec3 position)\n\
{\n\
	float s = dot(vec4(position, 1.0), u_FogDistance) * 8.0;\n\
	float t = dot(vec4(position, 1.0), u_FogDepth);\n\
	\n\
	float eyeOutside = float(u_FogEyeT < 0.0);\n\
	float fogged = float(t < eyeOutside);\n\
	\n\
	t += 1e-6;\n\
	t *= fogged / (t - u_FogEyeT * eyeOutside);\n\
	\n\
	return s * t;\n\
}\n\n";
	}

	// Main function body
	code += "void main() {\n";

	if ( permutation & GENERICDEF_USE_VERTEX_ANIMATION )
	{
		code += "	vec3 position = mix(attr_Position, attr_Position2, u_VertexLerp);\n";
		code += "	vec3 normal = mix(attr_Normal, attr_Normal2, u_VertexLerp);\n";
		code += "	normal = normalize(normal - vec3(0.5));\n";
	}
	else if ( permutation & GENERICDEF_USE_SKELETAL_ANIMATION )
	{
		code += "	vec4 position4 = vec4(0.0);\n\
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
		code += "	vec3 position = attr_Position;\n";
		code += "	vec3 normal = attr_Normal * 2.0 - vec3(1.0);\n";
	}

	code += "\n\
	gl_Position = u_ModelViewProjectionMatrix * vec4(position, 1.0);\n\
	\n\
	vec2 tex;\n";

	for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
	{
		const shaderStage_t *stage = shader->stages[i];


		code += "	var_TexCoords[";
		code += (i + '0');
		code += "] = attr_TexCoord0;\n";
	}

	if ( permutation & GENERICDEF_USE_LIGHTMAP )
	{
		code += "	var_LightTex = attr_TexCoord1;\n";
	}

	code += '\n';

	for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
	{
		code += "	var_Colors[";
		code += (i + '0');
		code += "] = u_VertColor * attr_Color + u_BaseColor;\n";
	}

	code += "}\n\n";
}

static void GenerateGenericFragmentShaderCode(
	const shader_t *shader,
	const char *defines[],
	uint32_t permutation,
	GLSLShader& genShader )
{
	std::string& code = genShader.code;

	// Determine uniforms needed
	memset( genShader.uniforms, 0, sizeof( genShader.uniforms ) );

	SetBit( genShader.uniforms, UNIFORM_DIFFUSEMAP );
	SetBit( genShader.samplers, TB_DIFFUSEMAP );
	
	if ( permutation & GENERICDEF_USE_LIGHTMAP )
	{
		SetBit( genShader.uniforms, UNIFORM_LIGHTMAP );
		SetBit( genShader.samplers, TB_LIGHTMAP );
	}

	#if 0
	if ( permutation & GENERICDEF_USE_NORMALMAP )
	{
		// Per unique normal map
		SetBit( genShader.uniforms, UNIFORM_NORMALMAP );
		SetBit( genShader.samplers, TB_NORMALMAP );
	}

	if ( permutation & GENERICDEF_USE_DELUXEMAP )
	{
		// Per unique deluxe map
		SetBit( genShader.uniforms, UNIFORM_DELUXEMAP );
		SetBit( genShader.samplers, TB_DELUXEMAP );
	}

	if ( permutation & GENERICDEF_USE_SPECULARMAP )
	{
		// Per unique deluxe map
		SetBit( genShader.uniforms, UNIFORM_SPECULARMAP );
		SetBit( genShader.samplers, TB_SPECULARMAP );
	}

	if ( permutation & GENERICDEF_USE_SHADOWMAP )
	{
		SetBit( genShader.uniforms, UNIFORM_SHADOWMAP );
		SetBit( genShader.samplers, TB_SHADOWMAP );
	}

	if ( permutation & GENERICDEF_USE_CUBEMAP )
	{
		SetBit( genShader.uniforms, UNIFORM_CUBEMAP );
		SetBit( genShader.samplers, TB_CUBEMAP );
	}

	if ( permutation & (GENERICDEF_USE_NORMALMAP | GENERICDEF_USE_SPECULARMAP | GENERICDEF_USE_CUBEMAP) )
	{
		SetBit( genShader.uniforms, UNIFORM_ENABLETEXTURES );
	}

	if ( permutation & (GENERICDEF_USE_LIGHTVECTOR | GENERICDEF_USE_FASTLIGHT) )
	{
		SetBit( genShader.uniforms, UNIFORM_DIRECTLIGHT );
		SetBit( genShader.uniforms, UNIFORM_AMBIENTLIGHT );
	}

	if ( permutation & (GENERICDEF_USE_PRIMARYLIGHT | GENERICDEF_USE_SHADOWMAP) )
	{
		SetBit( genShader.uniforms, UNIFORM_PRIMARYLIGHTCOLOR );
		SetBit( genShader.uniforms, UNIFORM_PRIMARYLIGHTAMBIENT );
	}

	if ( (permutation & GENERICDEF_USE_LIGHT) != 0 &&
			(permutation & GENERICDEF_USE_FASTLIGHT) == 0 )
	{
		SetBit( genShader.uniforms, UNIFORM_NORMALSCALE );
		SetBit( genShader.uniforms, UNIFORM_SPECULARSCALE );
	}

	if ( (permutation & GENERICDEF_USE_LIGHT) != 0 &&
			(permutation & GENERICDEF_USE_FASTLIGHT) == 0 &&
			(permutation & GENERICDEF_USE_CUBEMAP) != 0 )
	{
		SetBit( genShader.uniforms, UNIFORM_CUBEMAP );
		SetBit( genShader.samplers, TB_CUBEMAP );
	}
	#endif
	
	// Generate the code
	AddShaderHeaderCode( shader, defines, permutation, code );

	code += '\n';

	// Add uniforms
	for ( int i = 0; i < UNIFORM_COUNT; i++ )
	{
		if ( IsBitSet( genShader.uniforms, i ) )
		{
			code += "uniform ";
			code += glslTypeStrings[uniformsInfo[i].type];

			if ( uniformsInfo[i].size > 1 )
			{
				// TODO: Add array specifier
			}

			code += ' ';
			code += uniformsInfo[i].name;
			code += ";\n";
		}
	}

	code += '\n';

	// 
	// Add inputs
	//
	code += "in vec2 var_TexCoords[";
	code += (shader->numUnfoggedPasses + '0');
	code += "];\n";
	code += "in vec4 var_Color;\n";

	if ( permutation & GENERICDEF_USE_LIGHTMAP )
	{
		code += "in vec2 var_LightTex;\n";
	}

	code += '\n';

	//
	// Add outputs
	//
	code += "out vec4 out_Color;\n";
	if ( permutation & GENERICDEF_USE_GLOW_BUFFER )
	{
		code += "out vec4 out_Glow;\n";
	}

	//
	// Main function body
	//
	code += "void main() {\n\
	vec4 color = texture(u_DiffuseMap, var_TexCoords[0]);\n\
	out_Color = color * var_Color;\n\
	out_Glow = vec4(0.0);\n\
}";
}

uint32_t GetShaderCapabilities( const shader_t *shader )
{
	uint32_t caps = 0;

	if ( shader->numDeforms )
	{
		caps |= GENERICDEF_USE_DEFORM_VERTEXES;
	}

	for ( int i = 0; i < shader->numUnfoggedPasses; i++ )
	{
		const shaderStage_t *stage = shader->stages[i];

		if ( stage->bundle[1].image[0] && shader->multitextureEnv )
		{
			caps |= GENERICDEF_USE_LIGHTMAP;
		}

		if ( stage->rgbGen == CGEN_LIGHTING_DIFFUSE ||
				stage->alphaGen == AGEN_LIGHTING_SPECULAR ||
				stage->alphaGen == AGEN_PORTAL )
		{
			caps |= GENERICDEF_USE_RGBAGEN;
		}

		if ( stage->bundle[0].tcGen != TCGEN_TEXTURE ||
				stage->bundle[0].numTexMods > 0 )
		{
			caps |= GENERICDEF_USE_TCGEN_AND_TCMOD;
		}

		if ( stage->glow )
		{
			caps |= GENERICDEF_USE_GLOW_BUFFER;
		}
	}

	return caps;
}

void SetMaterialData( Material *material, void *newData, uniform_t uniform )
{
	ShaderProgram *program = material->program;
	ConstantDescriptor *constantData = &program->constants[program->uniformDataIndices[uniform]];
	void *data = reinterpret_cast<char *>(material->constantData) + constantData->offset;

	switch ( constantData->type )
	{
		case GLSL_INT:
			memcpy( data, newData, sizeof( int ) );
			break;

		case GLSL_FLOAT:
			memcpy( data, newData, sizeof( float ) );
			break;

		case GLSL_FLOAT5:
			memcpy( data, newData, sizeof( float ) * 5 );
			break;

		case GLSL_VEC2:
			memcpy( data, newData, sizeof( float ) * 2 );
			break;

		case GLSL_VEC3:
			memcpy( data, newData, sizeof( float ) * 3 );
			break;

		case GLSL_VEC4:
			memcpy( data, newData, sizeof( float ) * 4 );
			break;

		case GLSL_MAT16:
			memcpy( data, newData, sizeof( float ) * 16 );
			break;

		default:
			assert( false );
			break;
	}
}

static void FillDefaultUniformData( Material *material, const int uniformIndexes[UNIFORM_COUNT], const shader_t *shader )
{
	ShaderProgram *program = material->program;
	for ( int i = 0,  end = program->numUniforms; i < end; i++ )
	{
		const ConstantDescriptor *uniform = &program->constants[i];
		void *data = reinterpret_cast<char *>(material->constantData) + uniform->offset;

		switch ( uniformIndexes[i] )
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

			case UNIFORM_DEFORMGEN:
				*(int *)data = 0;
				break;

			case UNIFORM_DEFORMPARAMS:
				((float *)data)[0] = 0.0f;
				((float *)data)[1] = 0.0f;
				((float *)data)[2] = 0.0f;
				((float *)data)[3] = 0.0f;
				((float *)data)[4] = 0.0f;
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
				assert(false);
				break;
		}
	}
}

Material *GenerateGenericGLSLShader( const shader_t *shader, const char *defines[], uint32_t permutation )
{
	// Calculate hash for shader

	// shader program = HashTableFind( table, hash );
	// if ( shader program == NULL ) {
	GLSLShader vertexShader;
	GLSLShader fragmentShader;

	vertexShader.code.reserve( 16384 );
	fragmentShader.code.reserve( 16384 );

	permutation |= GetShaderCapabilities( shader );

	uint32_t sharedUniforms[UNIFORMS_BITFIELD_ARRAY_SIZE];
	int uniformIndexes[UNIFORM_COUNT];
	int numUniforms = 0;

	// Generate shader code
	GenerateGenericVertexShaderCode(
		shader,
		defines,
		permutation,
		vertexShader );

	GenerateGenericFragmentShaderCode(
		shader,
		defines,
		permutation,
		fragmentShader );

	for ( int i = 0; i < UNIFORMS_BITFIELD_ARRAY_SIZE; i++ )
	{
		sharedUniforms[i] = vertexShader.uniforms[i] | fragmentShader.uniforms[i];
	}

	// Generate static data and allocate memory for dynamic data

	// Need array of structs:
	//   uniform_t type;
	//   int offset;
	//   int sizeInFloats;

	// FIXME: Refactor into tr_glsl.cpp
	const char *vertexShaderCode = vertexShader.code.c_str();
	const char *fragmentShaderCode = fragmentShader.code.c_str();

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

	// For each vertex shader input {
	//   glBindAttribLocation(program, input location);
	// }

	qglLinkProgram( program );

	qglUseProgram( program );

	// Don't need to initialise defaults for the program.

	// FIXME: Lol, leak memory
	ShaderProgram *shaderProgram = new ShaderProgram();
	shaderProgram->program = program;
	shaderProgram->fragmentShader = fshader;
	shaderProgram->vertexShader = vshader;
	shaderProgram->numUniforms = numUniforms;
	shaderProgram->constants = new ConstantDescriptor[shaderProgram->numUniforms];

	int constantBufferSize = 0;
	for ( int i = 0, j = 0; i < numUniforms; i++ )
	{
		ConstantDescriptor *uniform = &shaderProgram->constants[uniformIndexes[i]];
		const uniformInfo_t *uniformInfo = &uniformsInfo[j];

		uniform->offset = constantBufferSize;
		uniform->size = uniformInfo->size;
		uniform->type = uniformInfo->type;
		uniform->location = qglGetUniformLocation( program, uniformInfo->name );

		shaderProgram->uniformDataIndices[i] = j;

		constantBufferSize += uniform->size;
	}

	// FIXME: Lol, more memory leaks
	Material *material = new Material();

	material->program = shaderProgram;
	material->constantData = malloc( constantBufferSize );

	FillDefaultUniformData( material, uniformIndexes, shader );
	
	return material;
}

void RB_BindMaterial( const Material *material )
{
	const ShaderProgram *program = material->program;

	qglUseProgram( program->program );

	// Update uniforms
	for ( int i = 0, end = program->numUniforms; i < end; i++ )
	{
		const ConstantDescriptor *constantData = &program->constants[i];
		const GLfloat *data = reinterpret_cast<const GLfloat *>(material->constantData) + constantData->offset;

		switch ( constantData->type )
		{
			case GLSL_INT:
				qglUniform1iv( constantData->location, constantData->size, reinterpret_cast<const GLint *>(data) );
				break;

			case GLSL_FLOAT:
				qglUniform1fv( constantData->location, constantData->size, data );
				break;

			case GLSL_FLOAT5:
				qglUniform1fv( constantData->location, 5 * constantData->size, data );
				break;

			case GLSL_VEC2:
				qglUniform2fv( constantData->location, constantData->size, data );
				break;

			case GLSL_VEC3:
				qglUniform3fv( constantData->location, constantData->size, data );
				break;

			case GLSL_VEC4:
				qglUniform4fv( constantData->location, constantData->size, data );
				break;

			case GLSL_MAT16:
				qglUniformMatrix4fv( constantData->location, constantData->size, GL_FALSE, data );
				break;

			default:
				assert( false );
				break;
		}
	}

	// Bind textures
	for ( int i = 0; i < program->numSamplers; i++ )
	{
		qglActiveTextureARB( GL_TEXTURE0 + i );
		qglBindTexture( program->samplers[i].target, material->textures[i] );
	}
}