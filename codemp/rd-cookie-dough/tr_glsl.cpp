/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2020, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "tr_local.h"

#include <cassert>
#include "glad.h"
#include "tr_glsl.h"

static constexpr char VERSION_STRING[] = "#version 430 core\n";
static struct 
{
	ShaderProgram fullscreenProgram;
	ShaderProgram mainProgram;
	ShaderProgram skyProgram;
} s_shaders;

static GLuint GLSL_CompileShader(GLenum shaderType, const char* code, const char** definitions, size_t definitionsCount)
{
	assert(definitionsCount <= 126);

	GLuint shader = qglCreateShader(shaderType);

	const char *shaderStrings[128];
	int stringCount = 0;
	shaderStrings[stringCount++] = VERSION_STRING;

	for (int i = 0; i < definitionsCount; ++i)
	{
		char defineStr[128];
		int len = Com_sprintf(defineStr, sizeof(defineStr), "#define %s\n", definitions[i]);

		char *define = reinterpret_cast<char*>(Hunk_AllocateTempMemory(len + 1));
		Q_strncpyz(define, defineStr, len + 1);

		shaderStrings[stringCount++] = define;
	}

	shaderStrings[stringCount++] = code;

	qglShaderSource(shader, stringCount, shaderStrings, nullptr);
	qglCompileShader(shader);

	for ( int i = 1; (i + 1) < stringCount; ++i)
	{
		Hunk_FreeTempMemory(const_cast<char*>(shaderStrings[i]));
	}

	GLint status;
	qglGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		GLint logLength;
		qglGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

		char *logText = reinterpret_cast<char*>(ri.Hunk_AllocateTempMemory(logLength));
		qglGetShaderInfoLog(shader, logLength, nullptr, logText);

		Com_Printf("Failed to compile shader: %s\n", logText);

		ri.Hunk_FreeTempMemory(logText);
	}

	return shader;
}

static GLuint GLSL_CreateProgram(const char* vertexShaderCode, const char* fragmentShaderCode, const char **definitions, size_t definitionsCount)
{
	GLuint vertexShader = GLSL_CompileShader(GL_VERTEX_SHADER, vertexShaderCode, definitions, definitionsCount);
	GLuint fragmentShader = GLSL_CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode, definitions, definitionsCount);

	GLuint program = qglCreateProgram();
	qglAttachShader(program, vertexShader);
	qglAttachShader(program, fragmentShader);
	qglLinkProgram(program);

	GLint linkStatus;
	qglGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE)
	{
		GLint logLength;
		qglGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		char *logText = reinterpret_cast<char*>(ri.Hunk_AllocateTempMemory(logLength));
		qglGetProgramInfoLog(program, logLength, nullptr, logText);

		Com_Printf("Failed to link program: %s\n", logText);

		ri.Hunk_FreeTempMemory(logText);
	}

	qglDetachShader(program, vertexShader);
	qglDetachShader(program, fragmentShader);
	qglDeleteShader(vertexShader);
	qglDeleteShader(fragmentShader);

	return program;
}

static GLuint GLSL_CreateProgram(const char* vertexShaderCode, const char* fragmentShaderCode)
{
	return GLSL_CreateProgram(vertexShaderCode, fragmentShaderCode, nullptr, 0);
}

static void GLSL_MainShader_Init()
{
	static constexpr const char VERTEX_SHADER[] = R"(
layout(std140, binding = 0) uniform View
{
	mat4 u_ProjectionMatrix;
};

layout(std430, binding = 0) buffer ModelMatrices
{
	mat4 u_ModelViewMatrix[];
};

layout(location = 0) uniform float u_PushConstants[128];

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec2 in_TexCoord0;
#if defined(MULTITEXTURE)
layout(location = 3) in vec2 in_TexCoord1;
#endif

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec2 out_TexCoord0;
layout(location = 2) out vec2 out_TexCoord1;

#define u_EntityIndex int(u_PushConstants[0])

void main()
{
	vec4 position = vec4(in_Position, 1.0);
#if defined(RENDER_SCENE)
	position = u_ModelViewMatrix[u_EntityIndex] * position;
#endif
	gl_Position = u_ProjectionMatrix * position;

	out_Color = in_Color;
	out_TexCoord0 = in_TexCoord0;
#if defined(MULTITEXTURE)
	out_TexCoord1 = in_TexCoord1;
#endif
}
)";

	static constexpr const char FRAGMENT_SHADER[] = R"(
layout(binding = 0) uniform sampler2D u_Texture0;
layout(binding = 1) uniform sampler2D u_Texture1;
layout(location = 0) uniform float u_PushConstants[128];

layout(location = 0) in vec4 in_Color;
layout(location = 1) in vec2 in_TexCoord0;
layout(location = 2) in vec2 in_TexCoord1;

layout(location = 0) out vec4 out_FragColor;

#define u_MultiplyTextures (u_PushConstants[1] > 0.0)
#define u_AlphaTestValue (u_PushConstants[2])
#define u_AlphaTestFunc (int(u_PushConstants[3]))

void main()
{
	vec4 color = texture(u_Texture0, in_TexCoord0);

#if defined(MULTITEXTURE)
	vec4 color1 = texture(u_Texture1, in_TexCoord1);
	color = mix(color + color1, color * color1, u_MultiplyTextures);
#endif

	color *= in_Color;

	bool alphaTestPasses[4] = {
		true,
		color.a < u_AlphaTestValue,
		color.a > u_AlphaTestValue,
		color.a >= u_AlphaTestValue,
	};
	if (!alphaTestPasses[u_AlphaTestFunc])
	{
		discard;
	}

	out_FragColor = color;
}
)";

	ShaderProgram* program = &s_shaders.mainProgram;

	program->permutationCount = MAIN_SHADER_PERMUTATION_COUNT;
	program->permutations = reinterpret_cast<int*>(
		Hunk_Alloc(sizeof(int) * program->permutationCount, h_low));
	Com_Memset(program->permutations, 0, sizeof(int) * program->permutationCount);

	program->permutations[0] = GLSL_CreateProgram(VERTEX_SHADER, FRAGMENT_SHADER);

	{
		const char *defines[] = {"RENDER_SCENE"};
		program->permutations[MAIN_SHADER_RENDER_SCENE] =
			GLSL_CreateProgram(VERTEX_SHADER, FRAGMENT_SHADER, defines, 1);
	}

	{
		const char *defines[] = {"RENDER_SCENE", "MULTITEXTURE"};
		program->permutations[MAIN_SHADER_RENDER_SCENE | MAIN_SHADER_MULTITEXTURE] =
			GLSL_CreateProgram(VERTEX_SHADER, FRAGMENT_SHADER, defines, 2);
	}
}

static void GLSL_SkyShader_Init()
{
	static constexpr const char VERTEX_SHADER[] = R"(
layout(std140, binding = 0) uniform View
{
	mat4 u_ProjectionMatrix;
};

layout(std430, binding = 0) buffer ModelMatrices
{
	mat4 u_ModelViewMatrix[];
};

layout(location = 0) in vec3 in_Position;
layout(location = 2) in vec2 in_TexCoord;

layout(location = 0) out vec2 out_TexCoord;

void main()
{
	vec4 position = u_ModelViewMatrix[2047] * vec4(in_Position, 1.0);
	gl_Position = u_ProjectionMatrix * position;
	gl_Position.w = gl_Position.z;  // force vertices to far plane

	out_TexCoord = in_TexCoord;
}
)";

	static constexpr const char FRAGMENT_SHADER[] = R"(
layout(binding = 0) uniform sampler2D u_Texture;

layout(location = 0) in vec2 in_TexCoord;

layout(location = 0) out vec4 out_FragColor;

void main()
{
	out_FragColor = texture(u_Texture, in_TexCoord);
}
)";

	ShaderProgram* program = &s_shaders.skyProgram;

	program->permutationCount = 1;
	program->permutations = reinterpret_cast<int*>(
		Hunk_Alloc(sizeof(int) * program->permutationCount, h_low));
	Com_Memset(program->permutations, 0, sizeof(int) * program->permutationCount);

	program->permutations[0] = GLSL_CreateProgram(VERTEX_SHADER, FRAGMENT_SHADER);
}

void GLSL_Init()
{
	GLSL_MainShader_Init();
	GLSL_SkyShader_Init();
}

static void GLSL_ReleaseShaderProgram(ShaderProgram* program)
{
	for (int i = 0; i < program->permutationCount; ++i)
	{
		if (program->permutations[i] > 0)
		{
			qglDeleteProgram(program->permutations[i]);
		}
	}
}

void GLSL_Shutdown()
{
	GLSL_ReleaseShaderProgram(&s_shaders.fullscreenProgram);
	GLSL_ReleaseShaderProgram(&s_shaders.mainProgram);
	GLSL_ReleaseShaderProgram(&s_shaders.skyProgram);
}

ShaderProgram GLSL_MainShader_GetHandle()
{
	return s_shaders.mainProgram;
}

ShaderProgram GLSL_SkyShader_GetHandle()
{
	return s_shaders.skyProgram;
}

void GLSL_FullscreenShader_Init()
{
	static constexpr char VERTEX_SHADER[] = R"(
layout(location = 0) out vec2 out_TexCoord;

void main() {
	// 0 -> (-1, -1)
	// 1 -> ( 3, -1)
	// 2 -> (-1,  3)
	gl_Position = vec4(
		float(4 * (gl_VertexID % 2) - 1),
		float(4 * (gl_VertexID / 2) - 1),
		0.0,
		1.0);
	
	// 0 -> (0, 0)
	// 1 -> (2, 0)
	// 2 -> (0, 2)
	out_TexCoord = vec2(
		float(2 * (gl_VertexID % 2)),
		float(1 - 2 * (gl_VertexID / 2)));
}
)";

	static constexpr char FRAGMENT_SHADER[] = R"(
layout(binding = 0) uniform sampler2D u_SplashImage;
layout(location = 0) in vec2 in_TexCoord;
layout(location = 0) out vec4 out_FragColor;

void main() {
	out_FragColor = texture(u_SplashImage, in_TexCoord);
}
)";

	ShaderProgram* program = &s_shaders.fullscreenProgram;

	program->permutationCount = 1;
	program->permutations = reinterpret_cast<int*>(
		Hunk_Alloc(sizeof(int) * program->permutationCount, h_low));
	Com_Memset(program->permutations, 0, sizeof(int) * program->permutationCount);

	program->permutations[0] = GLSL_CreateProgram(VERTEX_SHADER, FRAGMENT_SHADER);
}

ShaderProgram GLSL_FullscreenShader_GetHandle()
{
	return s_shaders.fullscreenProgram;
}
