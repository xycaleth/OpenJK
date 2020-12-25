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

#include "glad.h"

static struct {
	GLuint vbo;
	size_t offset;
	size_t size;

	GLuint ibo;
	size_t iboOffset;
	size_t iboSize;
} s_buffers;

void GpuBuffers_Init()
{
	s_buffers = {};
	qglGenBuffers(1, &s_buffers.vbo);
	qglGenBuffers(1, &s_buffers.ibo);
}

int GpuBuffers_AllocFrameVertexDataMemory(const void* data, size_t size)
{
	const size_t paddedSize = (size + 15) & ~15;

	qglBindBuffer(GL_ARRAY_BUFFER, s_buffers.vbo);
	if (s_buffers.size == 0 || (s_buffers.offset + paddedSize) >= s_buffers.size)
	{
		// 16mb for now
		qglBufferData(GL_ARRAY_BUFFER, 16 * 1024 * 1024, nullptr, GL_STREAM_DRAW);
		s_buffers.size = 16 * 1024 * 1024;
		s_buffers.offset = 0;
	}

	qglBufferSubData(GL_ARRAY_BUFFER, s_buffers.offset, size, data);

	int offset = s_buffers.offset;
	s_buffers.offset += paddedSize;
	return offset;
}

int GpuBuffers_AllocFrameIndexDataMemory(const void* data, size_t size)
{
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_buffers.ibo);
	if (s_buffers.iboSize == 0 || (s_buffers.iboOffset + size) >= s_buffers.iboSize)
	{
		// 4mb for now
		qglBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * 1024 * 1024, nullptr, GL_STREAM_DRAW);
		s_buffers.iboSize = 4 * 1024 * 1024;
		s_buffers.iboOffset = 0;
	}

	qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, s_buffers.iboOffset, size, data);

	int offset = s_buffers.iboOffset;
	s_buffers.iboOffset += size;
	return offset;
}
