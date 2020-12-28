#include "tr_gpucontext.h"

static struct RenderContext
{
	uint32_t enabledVertexAttribs;
	int shaderProgram;
	image_t *texture;

	int indexBuffer;

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

	qglBindBufferRange(GL_UNIFORM_BUFFER, 0, tr.viewConstantsBuffer, 0, sizeof(float) * 16);

	for ( int i = 0; i < drawItem->layerCount; ++i )
	{
		const DrawItem::Layer* layer = drawItem->layers + i;
		GL_State(layer->stateGroup.stateBits);

		if ( (layer->enabledVertexAttributes & 1) != 0 )
		{
			if ((s_context.enabledVertexAttribs & 1) == 0)
			{
				qglEnableVertexAttribArray(0);
				s_context.enabledVertexAttribs |= 1;
			}

			qglBindBuffer(GL_ARRAY_BUFFER, layer->vertexBuffers[0].handle);
			qglVertexAttribPointer(
				0, 3, GL_FLOAT, GL_FALSE, 16, reinterpret_cast<const void*>(layer->vertexBuffers[0].offset));
		}
		else
		{
			if (s_context.enabledVertexAttribs & 1)
			{
				qglDisableVertexAttribArray(0);
				s_context.enabledVertexAttribs &= ~1u;
			}
		}

		if ( (layer->enabledVertexAttributes & 2) != 0 )
		{
			if ((s_context.enabledVertexAttribs & 2) == 0)
			{
				qglEnableVertexAttribArray(1);
				s_context.enabledVertexAttribs |= 2;
			}

			qglBindBuffer(GL_ARRAY_BUFFER, layer->vertexBuffers[1].handle);
			qglVertexAttribPointer(
				1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, reinterpret_cast<const void*>(layer->vertexBuffers[1].offset));
		}
		else
		{
			if (s_context.enabledVertexAttribs & 2)
			{
				qglDisableVertexAttribArray(1);
				s_context.enabledVertexAttribs &= ~2u;
			}
		}

		if ( (layer->enabledVertexAttributes & 4) != 0 )
		{
			if ((s_context.enabledVertexAttribs & 4) == 0)
			{
				qglEnableVertexAttribArray(2);
				s_context.enabledVertexAttribs |= 4;
			}
			qglBindBuffer(GL_ARRAY_BUFFER, layer->vertexBuffers[2].handle);
			qglVertexAttribPointer(
				2, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(layer->vertexBuffers[2].offset));
		}
		else
		{
			if (s_context.enabledVertexAttribs & 4)
			{
				qglDisableVertexAttribArray(2);
				s_context.enabledVertexAttribs &= ~4u;
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

