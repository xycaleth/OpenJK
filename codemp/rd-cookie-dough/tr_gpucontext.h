#pragma once

#include "tr_local.h"

#include "tr_buffers.h"
#include "tr_glsl.h"
#include <cstdint>

enum PrimitiveType {
	PRIMITIVE_TRIANGLES
};

enum DrawType {
	DRAW_ARRAYS,
	DRAW_INDEXED,
};

struct StateGroup
{
	uint64_t stateBits;
};

struct DrawItem
{
	int layerCount;
	struct Layer
	{
		ShaderProgram shaderProgram;
		uint32_t shaderOptions;

		StateGroup stateGroup;
		image_t* textures[2];

		// Vertex format
		uint32_t enabledVertexAttributes;
		VertexBuffer vertexBuffers[4];

		uint32_t storageBuffersUsed;
		StorageBuffer storageBuffers[1];

		uint32_t constantBuffersUsed;
		ConstantBuffer constantBuffers[1];

		bool modulateTextures;
		float alphaTestValue;
		int alphaTestFunc;
	} layers[16];

	float minDepthRange;
	float maxDepthRange;

	bool isEntity;
	int entityNum;

	IndexBuffer indexBuffer;
	DrawType drawType;
	PrimitiveType primitiveType;
	int count;
	int offset;
};

void RenderContext_Init();
void RenderContext_Shutdown();

void RenderContext_AddDrawItem(const DrawItem& drawItem);
void RenderContext_Submit();
