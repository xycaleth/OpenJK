#pragma once

#include <cstddef>
#include <vector>

#include "qcommon/q_math.h"

typedef struct VBO_s VBO_t;
typedef struct IBO_s IBO_t;
typedef struct shader_s shader_t;
class Allocator;

namespace r2
{
    struct command_buffer_t;

    struct append_vertex_buffer_t
    {
        VBO_t *vbo;
        int cursor;
    };

    struct append_index_buffer_t
    {
        IBO_t *ibo;
        int cursor;
    };

    struct draw_batch_t
    {
        std::vector<byte> vertexData;
        std::vector<byte> indexData;

        int vertexCount;
        int indexCount;
    };

    struct frame_t
    {
        int number;

        append_vertex_buffer_t *vertexBuffer;
        append_index_buffer_t *indexBuffer;
        command_buffer_t *cmdBuffer;

        draw_batch_t quadsBatch;
        const shader_t *batchShader;

        vec4_t color;
        bool startedRenderPass;
        Allocator *memory;
    };

    struct buffer_slice_t
    {
        int offset;
        size_t size;
    };

    struct quad_vertex_t
    {
        vec2_t position;
        vec2_t texcoord;
        vec4_t color;
    };

    void FlushQuadsBatch(draw_batch_t *drawBatch);

    void ResetDrawBatch(draw_batch_t *drawBatch);

    void AppendDrawBatch(
        draw_batch_t *drawBatch,
        const void *vertexData,
        size_t vertexDataSize,
        int vertexCount,
        const void *indexData,
        size_t indexDataSize,
        int indexCount);

    buffer_slice_t AppendBuffer(
        append_vertex_buffer_t *vertexBuffer,
        const void *data,
        size_t sizeInBytes);

    buffer_slice_t AppendBuffer(
        append_index_buffer_t *indexBuffer,
        const void *data,
        size_t sizeInBytes);

    void InitFrame(frame_t *frame);

    void SubmitFrame(frame_t *frame);

    void FreeFrame(frame_t *frame);
}