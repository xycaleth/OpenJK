#pragma once

#include <cstdint>

#include "qcommon/q_math.h"

struct vertexAttribute_t;
struct UniformData;

typedef struct IBO_s IBO_t;
typedef struct image_s image_t;
typedef struct FBO_s FBO_t;
typedef struct shaderProgram_s shaderProgram_t;

namespace r2
{

    const int MAX_COLOR_ATTACHMENT_COUNT = 8;

    enum primitive_type_t
    {
        PRIMITIVE_TYPE_TRIANGLES = 0,
    };

    enum index_type_t
    {
        INDEX_TYPE_UINT16 = 0,
        INDEX_TYPE_UINT32
    };

    struct render_state_t
    {
        uint32_t stateBits;
        uint32_t cullType;
    };

    enum clear_action_t
    {
        CLEAR_ACTION_NONE,
        CLEAR_ACTION_FILL
    };

    struct rect_t
    {
        int x;
        int y;
        int width;
        int height;
    };

    struct render_pass_t
    {
        FBO_t *framebuffer;

        rect_t viewport;

        clear_action_t clearColorAction[MAX_COLOR_ATTACHMENT_COUNT];
        vec4_t clearColor[MAX_COLOR_ATTACHMENT_COUNT];

        clear_action_t clearDepthAction;
        float clearDepth;
    };

    enum command_type_t
    {
        CMD_TYPE_INVALID = 0,

        CMD_TYPE_DRAW,
        CMD_TYPE_DRAW_INDEXED,

        CMD_TYPE_BEGIN_RENDER_PASS,
        CMD_TYPE_END_RENDER_PASS,

        CMD_TYPE_SET_VERTEX_ATTRIBUTES,
        CMD_TYPE_SET_INDEX_BUFFER,
        CMD_TYPE_SET_SHADER_PROGRAM,
        CMD_TYPE_SET_TEXTURE,
        CMD_TYPE_SET_RENDER_STATE,

        CMD_TYPE_END
    };

    struct command_draw_indexed_t
    {
        command_type_t cmdType;

        primitive_type_t primitiveType;
        int indexCount;
        index_type_t indexType;
        int offset;
        int instanceCount;
        int baseVertex;
    };

    struct command_draw_t
    {
        command_type_t cmdType;

        primitive_type_t primitiveType;
        int vertexCount;
        int firstVertex;
        int instanceCount;
    };

    struct command_set_render_state_t
    {
        command_type_t cmdType;

        render_state_t renderState;
    };

    struct command_set_texture_t
    {
        command_type_t cmdType;

        int index;
        const image_t *image;
    };

    struct command_set_shader_program_t
    {
        command_type_t cmdType;

        const shaderProgram_t *shaderProgram;
        const UniformData *uniformData;
    };

    struct command_set_index_buffer_t
    {
        command_type_t cmdType;

        const IBO_t *indexBuffer;
    };

    struct command_set_vertex_attributes_t
    {
        command_type_t cmdType;

        int attributeCount;
        // array of vertexAttribute_t follow this struct
    };

    struct command_begin_render_pass_t
    {
        command_type_t cmdType;

        // Can probably make this a bit more memory efficient?
        render_pass_t renderPass;
    };

    const int COMMAND_BUFFER_SIZE = 8 * 1024 * 1024;
    struct command_buffer_t
    {
        char *cursor;
        char buffer[COMMAND_BUFFER_SIZE];
    };

    void ResetCmdBuffer(command_buffer_t *cmdBuffer);

    void CmdBeginRenderPass(
        command_buffer_t *cmdBuffer,
        const render_pass_t *renderPass);

    void CmdEndRenderPass(command_buffer_t *cmdBuffer);

    void CmdBeginTransformFeedback(
        command_buffer_t *cmdBuffer);

    void CmdEndTransformFeedback(
        command_buffer_t *cmdBuffer);

    void CmdSetRenderState(
        command_buffer_t *cmdBuffer,
        const render_state_t *renderState);

    void CmdSetIndexBuffer(
        command_buffer_t *cmdBuffer,
        const IBO_t *indexBuffer);

    void CmdSetShaderProgram(
        command_buffer_t *cmdBuffer,
        const shaderProgram_t *shaderProgram,
        const UniformData *uniformData = nullptr);

    void CmdSetTexture(
        command_buffer_t *cmdBuffer,
        int index,
        const image_t *image);

    void CmdSetVertexAttributes(
        command_buffer_t *cmdBuffer,
        int attributeCount,
        const vertexAttribute_t *vertexAttributes);

    void CmdDrawIndexed(
        command_buffer_t *cmdBuffer,
        primitive_type_t primitiveType,
        int indexCount,
        index_type_t indexType,
        int offset,
        int instanceCount,
        int baseVertex);

    void CmdDraw(
        command_buffer_t *cmdBuffer,
        primitive_type_t primitiveType,
        int vertexCount,
        int firstVertex,
        int instanceCount);

    void CmdPresent(command_buffer_t *cmdBuffer);
}
