#pragma once

#include "tr_local.h"

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
		StateGroup stateGroup;
		image_t* textures[2];

		// Vertex format
		uint32_t enabledVertexAttributes;
		int vtxBufferOffsets[3];
	} layers[16];

	int shaderProgram;

	DrawType drawType;
	PrimitiveType primitiveType;
	int count;
	int offset;
};

void RenderContext_Init();
void RenderContext_AddDrawItem(const DrawItem& drawItem);
void RenderContext_Draw(const DrawItem* drawItem);
