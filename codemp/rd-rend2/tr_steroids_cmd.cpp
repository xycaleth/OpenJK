#include "tr_steroids_cmd.h"

#include <cstdint>

#include "qcommon/q_shared.h"
#include "tr_local.h"

namespace r2
{
    namespace
    {
        template<typename T>
        void AddCommand(command_buffer_t *cmdBuffer, const T *cmd)
        {
            memcpy(cmdBuffer->cursor, cmd, sizeof(T));
            cmdBuffer->cursor += sizeof(T);
        }

        template<typename T, typename U>
        void AddCommandWithData(
            command_buffer_t *cmdBuffer,
            const T *cmd,
            const U *data,
            int count)
        {
            AddCommand(cmdBuffer, cmd);

            memcpy(cmdBuffer->cursor, data, sizeof(U) * count);
            cmdBuffer->cursor += sizeof(U) * count;
        }
    }

    void ResetCmdBuffer(command_buffer_t *cmdBuffer)
    {
        cmdBuffer->cursor = cmdBuffer->buffer;
    }

    void CmdBeginRenderPass(
        command_buffer_t *cmdBuffer,
        const render_pass_t *renderPass)
    {
        command_begin_render_pass_t cmd = {CMD_TYPE_BEGIN_RENDER_PASS};
        cmd.renderPass = *renderPass;

        AddCommand(cmdBuffer, &cmd);
    }

    void CmdEndRenderPass(command_buffer_t *cmdBuffer)
    {
    }

    void CmdDrawIndexed(
        command_buffer_t *cmdBuffer,
        primitive_type_t primitiveType,
        int indexCount,
        index_type_t indexType,
        int offset,
        int instanceCount,
        int baseVertex)
    {
        command_draw_indexed_t cmd = {CMD_TYPE_DRAW_INDEXED};
        cmd.primitiveType = primitiveType;
        cmd.indexCount = indexCount;
        cmd.indexType = indexType;
        cmd.offset = offset;
        cmd.instanceCount = instanceCount;
        cmd.baseVertex = baseVertex;

        AddCommand(cmdBuffer, &cmd);
    }

    void CmdDraw(
        command_buffer_t *cmdBuffer,
        primitive_type_t primitiveType,
        int vertexCount,
        int firstVertex,
        int instanceCount)
    {
        command_draw_t cmd = {CMD_TYPE_DRAW};
        cmd.primitiveType = primitiveType;
        cmd.vertexCount = vertexCount;
        cmd.firstVertex = firstVertex;
        cmd.instanceCount = instanceCount;

        AddCommand(cmdBuffer, &cmd);
    }

    void CmdSetRenderState(
        command_buffer_t *cmdBuffer,
        const render_state_t *renderState)
    {
        command_set_render_state_t cmd = {CMD_TYPE_SET_RENDER_STATE};
        cmd.renderState = *renderState;

        AddCommand(cmdBuffer, &cmd);
    }

    void CmdSetShaderProgram(
        command_buffer_t *cmdBuffer,
        const shaderProgram_t *shaderProgram)
    {
        command_set_shader_program_t cmd = {CMD_TYPE_SET_SHADER_PROGRAM};
        cmd.shaderProgram = shaderProgram;
        cmd.uniformDataCount = 0;

        AddCommand(cmdBuffer, &cmd);
    }

    void CmdSetShaderProgram(
        command_buffer_t *cmdBuffer,
        const shaderProgram_t *shaderProgram,
        int uniformDataCount,
        const UniformData **uniformData)
    {
        command_set_shader_program_t cmd = {CMD_TYPE_SET_SHADER_PROGRAM};
        cmd.shaderProgram = shaderProgram;
        cmd.uniformDataCount = uniformDataCount;

        AddCommandWithData(cmdBuffer, &cmd, uniformData, uniformDataCount);
    }

    void CmdSetIndexBuffer(
        command_buffer_t *cmdBuffer,
        const IBO_t *indexBuffer)
    {
        command_set_index_buffer_t cmd = {CMD_TYPE_SET_INDEX_BUFFER};
        cmd.indexBuffer = indexBuffer;

        AddCommand(cmdBuffer, &cmd);
    }

    void CmdSetVertexAttributes(
        command_buffer_t *cmdBuffer,
        int attributeCount,
        const vertexAttribute_t *vertexAttributes)
    {
        command_set_vertex_attributes_t cmd = {CMD_TYPE_SET_VERTEX_ATTRIBUTES};
        cmd.attributeCount = attributeCount;

        AddCommandWithData(cmdBuffer, &cmd, vertexAttributes, attributeCount);
    }

    void CmdSetTexture(
        command_buffer_t *cmdBuffer,
        int index,
        const image_t *image)
    {
        command_set_texture_t cmd = {CMD_TYPE_SET_TEXTURE};
        cmd.index = index;
        cmd.image = image;

        AddCommand(cmdBuffer, &cmd);
    }

    void CmdPresent(command_buffer_t *cmdBuffer)
    {
        command_type_t cmd = CMD_TYPE_END;
        AddCommand(cmdBuffer, &cmd);
    }
};