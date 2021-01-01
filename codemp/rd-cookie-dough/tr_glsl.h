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
#pragma once

struct ShaderProgram
{
	int permutationCount;
	int *permutations;
};

enum MainShaderOptions
{
	MAIN_SHADER_RENDER_SCENE = (1u << 0),
	MAIN_SHADER_MULTITEXTURE = (1u << 1),

	MAIN_SHADER_PERMUTATION_COUNT = (1u << 2),
};

void GLSL_Init();
void GLSL_Shutdown();

void GLSL_FullscreenShader_Init();

ShaderProgram GLSL_MainShader_GetHandle();
ShaderProgram GLSL_SkyShader_GetHandle();
ShaderProgram GLSL_FullscreenShader_GetHandle();

