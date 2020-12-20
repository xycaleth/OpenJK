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

#include "glad.h"
#include "tr_glsl.h"

static constexpr char VERSION_STRING[] = "#version 430 core";
static struct 
{
	GLuint fullscreenProgram;
} s_shaders;

void GLSL_Init()
{
	static constexpr char VERTEX_SHADER[] = R"(
)";
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

	const char *vertexShaderStrings[] = {VERSION_STRING, VERTEX_SHADER};
	GLuint vertexShader = qglCreateShader(GL_VERTEX_SHADER);
	qglShaderSource(vertexShader, 2, vertexShaderStrings, nullptr);
	qglCompileShader(vertexShader);

	GLint status;
	qglGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		GLint logLength;
		qglGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);

		char *logText = reinterpret_cast<char*>(ri.Hunk_AllocateTempMemory(logLength));
		qglGetShaderInfoLog(vertexShader, logLength, nullptr, logText);

		Com_Printf("Failed to compile shader: %s\n", logText);

		ri.Hunk_FreeTempMemory(logText);
	}

	const char *fragmentShaderStrings[] = {VERSION_STRING, FRAGMENT_SHADER};
	GLuint fragmentShader = qglCreateShader(GL_FRAGMENT_SHADER);
	qglShaderSource(fragmentShader, 2, fragmentShaderStrings, nullptr);
	qglCompileShader(fragmentShader);

	qglGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		GLint logLength;
		qglGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);

		char *logText = reinterpret_cast<char*>(ri.Hunk_AllocateTempMemory(logLength));
		qglGetShaderInfoLog(fragmentShader, logLength, nullptr, logText);

		Com_Printf("Failed to compile shader: %s\n", logText);

		ri.Hunk_FreeTempMemory(logText);
	}

	GLuint program = qglCreateProgram();
	qglAttachShader(program, vertexShader);
	qglAttachShader(program, fragmentShader);
	qglLinkProgram(program);

	qglGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		GLint logLength;
		qglGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		char *logText = reinterpret_cast<char*>(ri.Hunk_AllocateTempMemory(logLength));
		qglGetProgramInfoLog(program, logLength, nullptr, logText);

		Com_Printf("Failed to link program: %s\n", logText);

		ri.Hunk_FreeTempMemory(logText);
	}

	qglDeleteShader(vertexShader);
	qglDeleteShader(fragmentShader);

	s_shaders.fullscreenProgram = program;
}

void GLSL_FullscreenShader_Use()
{
	qglUseProgram(s_shaders.fullscreenProgram);
}
