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

	int indexBuffer;
	int uniformBuffers[1];

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
	if (drawItem->shaderProgram != s_context.shaderProgram)
	{
		qglUseProgram(drawItem->shaderProgram);
		s_context.shaderProgram = drawItem->shaderProgram;
	}

	if (tr.viewConstantsBuffer != s_context.uniformBuffers[0])
	{
		qglBindBufferRange(GL_UNIFORM_BUFFER, 0, tr.viewConstantsBuffer, 0, sizeof(float) * 16);
		s_context.uniformBuffers[0] = tr.viewConstantsBuffer;
	}

	for ( int i = 0; i < drawItem->layerCount; ++i )
	{
		const DrawItem::Layer* layer = drawItem->layers + i;
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

