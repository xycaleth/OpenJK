#include "tr_gpucontext.h"

struct VertexBuffers
{
	int vertexBuffer;
	int offset;
};

static struct RenderContext
{
	uint32_t enabledVertexAttribs;
	VertexBuffers vertexBuffers[8];
	int shaderProgram;
	image_t *textures[2];

	float minDepthRange;
	float maxDepthRange;

	int indexBuffer;
	ConstantBuffer uniformBuffers[1];
	StorageBuffer storageBuffers[1];

	int drawItemCount;
	int drawItemCapacity;
	DrawItem* drawItems;
} s_context;

void RenderContext_Init()
{
	Com_Memset(&s_context, 0, sizeof(s_context));
	s_context.drawItemCapacity = 400;
	s_context.drawItems = reinterpret_cast<DrawItem*>(
		Z_Malloc(sizeof(DrawItem) * s_context.drawItemCapacity, TAG_GENERAL));
}

void RenderContext_Shutdown()
{
	if (s_context.drawItems != nullptr)
	{
		Z_Free(s_context.drawItems);
		s_context.drawItems = nullptr;
	}
}

void RenderContext_AddDrawItem(const DrawItem& drawItem)
{
	if (s_context.drawItemCount == s_context.drawItemCapacity)
	{
		const int newCapacity = s_context.drawItemCapacity * 1.5f;
		DrawItem* newDrawItems = reinterpret_cast<DrawItem*>(
			Z_Malloc(sizeof(DrawItem) * newCapacity, TAG_GENERAL));

		Com_Memcpy(newDrawItems, s_context.drawItems, sizeof(DrawItem) * s_context.drawItemCapacity);

		Z_Free(s_context.drawItems);
		s_context.drawItems = newDrawItems;
		s_context.drawItemCapacity = newCapacity;
	}

	s_context.drawItems[s_context.drawItemCount++] = drawItem;
}

static void RenderContext_Draw(const DrawItem* drawItem)
{
	if (drawItem->minDepthRange != s_context.minDepthRange ||
		drawItem->maxDepthRange != s_context.maxDepthRange)
	{
		qglDepthRangef(drawItem->minDepthRange, drawItem->maxDepthRange);
		s_context.minDepthRange = drawItem->minDepthRange;
		s_context.maxDepthRange = drawItem->maxDepthRange;
	}

	for ( int i = 0; i < drawItem->layerCount; ++i )
	{
		const DrawItem::Layer* layer = drawItem->layers + i;

		const int shaderProgram = layer->shaderProgram.permutations[layer->shaderOptions];
		assert(shaderProgram != 0);
		if (shaderProgram != s_context.shaderProgram)
		{
			qglUseProgram(shaderProgram);
			s_context.shaderProgram = shaderProgram;
		}
		
		if (drawItem->isEntity)
		{
			float pushConstants[128];
			pushConstants[0] = (float)drawItem->entityNum;
			if (layer->modulateTextures) 
			{
				pushConstants[1] = 1.0f;
			}
			qglUniform1fv(0, 2, pushConstants);
		}

		if (layer->constantBuffersUsed)
		{
			if (layer->constantBuffers[0].handle != s_context.uniformBuffers[0].handle ||
				layer->constantBuffers[0].offset != s_context.uniformBuffers[0].offset ||
				layer->constantBuffers[0].size != s_context.uniformBuffers[0].size)
			{
				qglBindBufferRange(
					GL_UNIFORM_BUFFER, 0, layer->constantBuffers[0].handle, 0, layer->constantBuffers[0].size);
				s_context.uniformBuffers[0] = layer->constantBuffers[0];
			}
		}

		if (layer->storageBuffersUsed)
		{
			if ((layer->storageBuffers[0].handle != s_context.storageBuffers[0].handle) ||
				(layer->storageBuffers[0].offset != s_context.storageBuffers[0].offset) ||
				(layer->storageBuffers[0].size != s_context.storageBuffers[0].size))
			{
				const StorageBuffer* buffer = layer->storageBuffers + 0;
				qglBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffer->handle, buffer->offset, buffer->size);
				s_context.storageBuffers[0] = *buffer;
			}
		}

		GL_State(layer->stateGroup.stateBits);

		int strides[] = {16, 4, 8, 8};
		for ( int attribIndex = 0; attribIndex < 4; ++attribIndex)
		{
			const uint32_t attribBit = 1u << attribIndex;
			if ( (layer->enabledVertexAttributes & attribBit) != 0 )
			{
				if ( (s_context.enabledVertexAttribs & attribBit) == 0 )
				{
					qglEnableVertexAttribArray(attribIndex);
					s_context.enabledVertexAttribs |= attribBit;
				}

				const VertexBuffer *vertexBuffer = layer->vertexBuffers + attribIndex;
				if (vertexBuffer->handle != s_context.vertexBuffers[attribIndex].vertexBuffer ||
					vertexBuffer->offset != s_context.vertexBuffers[attribIndex].offset)
				{
					qglBindVertexBuffer(
						attribIndex, vertexBuffer->handle, vertexBuffer->offset, strides[attribIndex]);

					s_context.vertexBuffers[attribIndex].vertexBuffer = vertexBuffer->handle;
					s_context.vertexBuffers[attribIndex].offset = vertexBuffer->offset;
				}
			}
			else
			{
				if ( s_context.enabledVertexAttribs & attribBit )
				{
					qglDisableVertexAttribArray(attribIndex);
					s_context.enabledVertexAttribs &= ~attribBit;
				}
			}
		}

		for (int i = 0; i < 2; ++i)
		{
			if (layer->textures[i] != nullptr &&
				layer->textures[i] != s_context.textures[i])
			{
				qglBindTextureUnit(i, layer->textures[i]->texnum);
				s_context.textures[i] = layer->textures[i];
			}
		}

		switch (drawItem->drawType)
		{
			case DRAW_ARRAYS:
				qglDrawArrays(
					GL_TRIANGLES,
					drawItem->offset,
					drawItem->count);
				break;
			case DRAW_INDEXED:
				if (drawItem->indexBuffer.handle != s_context.indexBuffer)
				{
					qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawItem->indexBuffer.handle);
					s_context.indexBuffer = drawItem->indexBuffer.handle;
				}

				qglDrawElements(
					GL_TRIANGLES,
					drawItem->count,
					GL_INDEX_TYPE,
					reinterpret_cast<const void*>(drawItem->indexBuffer.offset));
				break;
		}
	}
}

void RenderContext_Submit()
{
	for (int i = 0; i < s_context.drawItemCount; ++i)
	{
		RenderContext_Draw(s_context.drawItems + i);
	}
	s_context.drawItemCount = 0;
}
