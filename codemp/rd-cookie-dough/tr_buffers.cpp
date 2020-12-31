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

#include "tr_buffers.h"
#include "tr_local.h"

#include "glad.h"

#include "qcommon/q_shared.h"
#include <cstdint>

static struct {
	GLuint vbo;
	size_t vboOffset;
	size_t vboSize;
	void *vboBasePtr;

	GLuint ibo;
	size_t iboOffset;
	size_t iboSize;
	void *iboBasePtr;

	GLuint ubo;
	int uboAlignment;
	size_t uboOffset;
	size_t uboSize;
	void *uboBasePtr;

	GLuint ssbo;
	int ssboAlignment;
	size_t ssboOffset;
	size_t ssboSize;
	void *ssboBasePtr;
} s_buffers;

void GpuBuffers_Init()
{
	s_buffers = {};
	qglGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &s_buffers.uboAlignment);
	qglGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &s_buffers.ssboAlignment);

	int maxBufferSize;
	qglGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxBufferSize);

	ri.Printf(PRINT_ALL, "GL_MAX_UNIFORM_BLOCK_SIZE: %d\n", maxBufferSize);
}

void GpuBuffers_Shutdown()
{
	if (s_buffers.vbo != 0)
	{
		qglUnmapNamedBuffer(s_buffers.vbo);
		qglDeleteBuffers(1, &s_buffers.vbo);
	}

	if (s_buffers.ibo != 0)
	{
		qglUnmapNamedBuffer(s_buffers.ibo);
		qglDeleteBuffers(1, &s_buffers.ibo);
	}

	if (s_buffers.ubo != 0)
	{
		qglUnmapNamedBuffer(s_buffers.ubo);
		qglDeleteBuffers(1, &s_buffers.ubo);
	}

	if (s_buffers.ssbo != 0)
	{
		qglUnmapNamedBuffer(s_buffers.ssbo);
		qglDeleteBuffers(1, &s_buffers.ssbo);
	}
}

VertexBuffer GpuBuffers_AllocFrameVertexDataMemory(const void* data, size_t size)
{
	const size_t paddedSize = (size + 15) & ~15;

	if (s_buffers.vboSize == 0)
	{
		// 16mb for now
		const uint32_t flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		s_buffers.vboSize = 16 * 1024 * 1024;

		qglCreateBuffers(1, &s_buffers.vbo);
		qglNamedBufferStorage(s_buffers.vbo, s_buffers.vboSize, nullptr, flags);

		s_buffers.vboBasePtr = qglMapNamedBufferRange(s_buffers.vbo, 0, s_buffers.vboSize, flags);
	}
	
	if ((s_buffers.vboOffset + paddedSize) >= s_buffers.vboSize)
	{
		// This is completely wrong right now.
		s_buffers.vboOffset = 0;
	}

	if (data != nullptr)
	{
		Com_Memcpy(reinterpret_cast<char*>(s_buffers.vboBasePtr) + s_buffers.vboOffset, data, size);
	}

	VertexBuffer buffer = {};
	buffer.handle = s_buffers.vbo;
	buffer.offset = s_buffers.vboOffset;
	buffer.size = paddedSize;
	buffer.memory = reinterpret_cast<char*>(s_buffers.vboBasePtr) + s_buffers.vboOffset;

	s_buffers.vboOffset += paddedSize;

	return buffer;
}

IndexBuffer GpuBuffers_AllocFrameIndexDataMemory(const void* data, size_t size)
{
	if (s_buffers.iboSize == 0)
	{
		// 16mb for now
		const uint32_t flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		s_buffers.iboSize = 4 * 1024 * 1024;

		qglCreateBuffers(1, &s_buffers.ibo);
		qglNamedBufferStorage(s_buffers.ibo, s_buffers.iboSize, nullptr, flags);

		s_buffers.iboBasePtr = qglMapNamedBufferRange(s_buffers.ibo, 0, s_buffers.iboSize, flags);
	}
	
	if ((s_buffers.iboOffset + size) >= s_buffers.iboSize)
	{
		// This is completely wrong right now.
		s_buffers.iboOffset = 0;
	}

	if (data != nullptr)
	{
		Com_Memcpy(reinterpret_cast<char*>(s_buffers.iboBasePtr) + s_buffers.iboOffset, data, size);
	}

	IndexBuffer indexBuffer = {};
	indexBuffer.handle = s_buffers.ibo;
	indexBuffer.size = size;
	indexBuffer.offset = s_buffers.iboOffset;
	indexBuffer.memory = reinterpret_cast<char*>(s_buffers.iboBasePtr) + s_buffers.iboOffset;

	s_buffers.iboOffset += size;

	return indexBuffer;
}

ConstantBuffer GpuBuffers_AllocFrameConstantDataMemory(const void* data, size_t size)
{
	const size_t paddedSize = (size + s_buffers.uboAlignment) & ~s_buffers.uboAlignment;
	if (s_buffers.uboSize == 0)
	{
		// 4mb for now
		const uint32_t flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		s_buffers.uboSize = 4 * 1024 * 1024;

		qglCreateBuffers(1, &s_buffers.ubo);
		qglNamedBufferStorage(s_buffers.ubo, s_buffers.uboSize, nullptr, flags);

		s_buffers.uboBasePtr = qglMapNamedBufferRange(s_buffers.ubo, 0, s_buffers.uboSize, flags);
	}

	if ( (s_buffers.uboOffset + paddedSize) >= s_buffers.uboSize )
	{
		s_buffers.uboOffset = 0;
	}

	if (data != nullptr)
	{
		Com_Memcpy(reinterpret_cast<char*>(s_buffers.uboBasePtr) + s_buffers.uboOffset, data, size);
	}

	ConstantBuffer buffer = {};
	buffer.handle = s_buffers.ubo;
	buffer.offset = s_buffers.uboOffset;
	buffer.size = size;
	buffer.memory = reinterpret_cast<char*>(s_buffers.uboBasePtr) + s_buffers.uboOffset;

	s_buffers.uboOffset += paddedSize;

	return buffer;
}

StorageBuffer GpuBuffers_AllocFrameStorageDataMemory(const void* data, size_t size)
{
	const size_t paddedSize = (size + s_buffers.ssboAlignment) & ~s_buffers.ssboAlignment;
	if (s_buffers.ssboSize == 0)
	{
		// 4mb for now
		const uint32_t flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		s_buffers.ssboSize = 4 * 1024 * 1024;

		qglCreateBuffers(1, &s_buffers.ssbo);
		qglNamedBufferStorage(s_buffers.ssbo, s_buffers.ssboSize, nullptr, flags);

		s_buffers.ssboBasePtr = qglMapNamedBufferRange(s_buffers.ssbo, 0, s_buffers.ssboSize, flags);
	}

	if ( (s_buffers.ssboOffset + paddedSize) >= s_buffers.ssboSize )
	{
		s_buffers.ssboOffset = 0;
	}

	Com_Memcpy(reinterpret_cast<char*>(s_buffers.ssboBasePtr) + s_buffers.ssboOffset, data, size);

	StorageBuffer buffer = {};
	buffer.handle = s_buffers.ssbo;
	buffer.offset = s_buffers.ssboOffset;
	buffer.size = size;

	s_buffers.ssboOffset += paddedSize;

	return buffer;
}

ConstantBuffer GpuBuffers_AllocConstantDataMemory(const void* data, size_t size)
{
	GLuint ubo;
	qglCreateBuffers(1, &ubo);
	qglNamedBufferStorage(ubo, size, data, 0);

	ConstantBuffer buffer = {};
	buffer.handle = ubo;
	buffer.size = size;
	buffer.offset = 0;
	return buffer;
}

void GpuBuffers_ReleaseConstantDataMemory(const ConstantBuffer *buffer)
{
	GLuint ubo = buffer->handle;
	qglDeleteBuffers(1, &ubo);
}
