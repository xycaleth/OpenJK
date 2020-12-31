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
	image_t *texture;

	float minDepthRange;
	float maxDepthRange;

	int indexBuffer;
	ConstantBuffer uniformBuffers[1];
	StorageBuffer storageBuffers[1];

	int drawItemCount;
	DrawItem drawItems[4000];
} s_context;

void RenderContext_Init()
{
	Com_Memset(&s_context, 0, sizeof(s_context));
}

void RenderContext_AddDrawItem(const DrawItem& drawItem)
{
	s_context.drawItems[s_context.drawItemCount++] = drawItem;
}

void RenderContext_Draw(const DrawItem* drawItem)
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
		if (layer->shaderProgram != s_context.shaderProgram)
		{
			qglUseProgram(layer->shaderProgram);
			s_context.shaderProgram = layer->shaderProgram;
		}
		
		if (drawItem->isEntity)
		{
			float pushConstants[128];
			pushConstants[0] = (float)drawItem->entityNum;
			qglUniform1fv(0, 1, pushConstants);
		}

		if (backEnd.viewConstantsBuffer.handle != s_context.uniformBuffers[0].handle ||
			backEnd.viewConstantsBuffer.offset != s_context.uniformBuffers[0].offset ||
			backEnd.viewConstantsBuffer.size != s_context.uniformBuffers[0].size)
		{
			qglBindBufferRange(GL_UNIFORM_BUFFER, 0, backEnd.viewConstantsBuffer.handle, 0, backEnd.viewConstantsBuffer.size);
			s_context.uniformBuffers[0] = backEnd.viewConstantsBuffer;
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

		int strides[] = {16, 4, 8};
		for ( int attribIndex = 0; attribIndex < 3; ++attribIndex)
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

		if (layer->textures[0] != s_context.texture)
		{
			GL_Bind(layer->textures[0]);
			s_context.texture = layer->textures[0];
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

