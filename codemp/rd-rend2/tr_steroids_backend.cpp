#include "tr_steroids_backend.h"
#include "tr_steroids_cmd.h"

#include "qgl.h"
#include "tr_local.h"

namespace r2
{
    namespace
    {
        GLenum GLPrimitiveType(primitive_type_t primitiveType)
        {
            switch (primitiveType)
            {
                case PRIMITIVE_TYPE_TRIANGLES: return GL_TRIANGLES;
            }
        }

        GLenum GLIndexType(index_type_t indexType)
        {
            switch (indexType)
            {
                case INDEX_TYPE_UINT16: return GL_UNSIGNED_SHORT;
                case INDEX_TYPE_UINT32: return GL_UNSIGNED_INT;
            }
        }

        template<typename T>
        const T *GetCommand(const char **cursor)
        {
            const T *cmd = reinterpret_cast<const T *>(*cursor);
            *cursor += sizeof(T);
            return cmd;
        }

        template<typename T>
        const T *GetCommandData(const char **cursor, int elementCount)
        {
            const T *data = reinterpret_cast<const T *>(*cursor);
            *cursor += sizeof(T) * elementCount;
            return data;
        }
    }

    void SubmitCommands(const command_buffer_t *cmdBuffer)
    {
        const char *cursor = cmdBuffer->buffer;

        command_type_t cmdType;

        const int submitStartTime = ri.Milliseconds();
        int drawCount = 0;

        do
        {
            cmdType = *reinterpret_cast<const command_type_t *>(cursor);
            switch (cmdType)
            {
                case CMD_TYPE_INVALID:
                {
                    ri.Printf(
                        PRINT_ERROR,
                        "Invalid render command: %d",
                        cmdType);

                    cmdType = CMD_TYPE_END;
                    break;
                }

                case CMD_TYPE_DRAW:
                {
                    const auto *cmd = GetCommand<command_draw_t>(&cursor);

                    qglDrawArraysInstanced(
                        GLPrimitiveType(cmd->primitiveType),
                        cmd->firstVertex,
                        cmd->vertexCount,
                        cmd->instanceCount);

                    ++drawCount;
                    break;
                }

                case CMD_TYPE_DRAW_INDEXED:
                {
                    const auto *cmd =
                        GetCommand<command_draw_indexed_t>(&cursor);

                    qglDrawElementsInstancedBaseVertex(
                        GLPrimitiveType(cmd->primitiveType),
                        cmd->indexCount,
                        GLIndexType(cmd->indexType),
                        reinterpret_cast<const void *>(cmd->offset),
                        cmd->instanceCount,
                        cmd->baseVertex);

                    ++drawCount;
                    break;
                }

                case CMD_TYPE_BEGIN_RENDER_PASS:
                {
                    const auto *cmd =
                        GetCommand<command_begin_render_pass_t>(&cursor);

                    const auto *renderPass = &cmd->renderPass;
                    FBO_Bind(renderPass->framebuffer);

                    // TODO: Support MRTs
                    if (renderPass->clearColorAction[0] == CLEAR_ACTION_FILL)
                    {
                        qglClearBufferfv(
                            GL_COLOR,
                            0,
                            renderPass->clearColor[0]);
                    }

                    if (renderPass->clearDepth == CLEAR_ACTION_FILL)
                    {
                        qglClearBufferfv(GL_DEPTH, 0, &renderPass->clearDepth);
                    }

                    qglViewport(
                        renderPass->viewport.x,
                        renderPass->viewport.y,
                        renderPass->viewport.width,
                        renderPass->viewport.height);
                    break;
                }

                case CMD_TYPE_END_RENDER_PASS:
                {
                    // Resolve MSAA at some point
                    break;
                }

                case CMD_TYPE_SET_VERTEX_ATTRIBUTES:
                {
                    const auto *cmd =
                        GetCommand<command_set_vertex_attributes_t>(&cursor);

                    const auto *vertexAttributes =
                        GetCommandData<vertexAttribute_t>(
                            &cursor,
                            cmd->attributeCount);

                    GL_VertexAttribPointers(
                        static_cast<size_t>(cmd->attributeCount),
                        vertexAttributes);
                    break;
                }

                case CMD_TYPE_SET_INDEX_BUFFER:
                {
                    const auto *cmd =
                        GetCommand<command_set_index_buffer_t>(&cursor);

                    R_BindIBO(cmd->indexBuffer);
                    break;
                }

                case CMD_TYPE_SET_TEXTURE:
                {
                    const auto *cmd =
                        GetCommand<command_set_texture_t>(&cursor);

                    // Unfortunate const_cast...
                    GL_BindToTMU(const_cast<image_t *>(cmd->image), cmd->index);
                    break;
                }

                case CMD_TYPE_SET_SHADER_PROGRAM:
                {
                    const auto *cmd =
                        GetCommand<command_set_shader_program_t>(&cursor);

                    GLSL_BindProgram(cmd->shaderProgram);

                    if (cmd->uniformData != nullptr)
                    {
                        GLSL_SetUniforms(
                            const_cast<shaderProgram_t *>(cmd->shaderProgram),
                            cmd->uniformData);
                    }
                    break;
                }

                case CMD_TYPE_SET_RENDER_STATE:
                {
                    const auto *cmd =
                        GetCommand<command_set_render_state_t>(&cursor);

                    GL_State(cmd->renderState.stateBits);
                    GL_Cull(cmd->renderState.cullType);
                    break;
                }

                case CMD_TYPE_END:
                {
                    ri.WIN_Present(&window);
                    break;
                }
            }
        }
        while (cmdType != CMD_TYPE_END);

        const int submitEndTime = ri.Milliseconds();
        const int submitTimeInterval = submitEndTime - submitStartTime;

        if (r_speeds->integer == 20)
        {
            ri.Printf(
                PRINT_ALL,
                "CPU Draw Submit Time: %dms for %d draw calls\n",
                submitTimeInterval,
                drawCount);
        }
    }
}