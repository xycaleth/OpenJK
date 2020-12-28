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

void GpuBuffers_Init();
void GpuBuffers_Shutdown();

int GpuBuffers_AllocFrameVertexDataMemory(const void* data, size_t size);
int GpuBuffers_AllocFrameIndexDataMemory(const void* data, size_t size);
int GpuBuffers_AllocFrameConstantDataMemory(const void* data, size_t size);

int GpuBuffers_AllocConstantDataMemory(const void *data, size_t size);
void GpuBuffers_ReleaseConstantDataMemory(int buffer);

void GpuBuffers_BindConstantBuffer(int bufferIndex, int offset, int size);
