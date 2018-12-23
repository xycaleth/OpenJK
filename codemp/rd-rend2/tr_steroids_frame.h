#pragma once

#include <cstddef>

#include "qcommon/q_math.h"

typedef struct VBO_s VBO_t;
typedef struct IBO_s IBO_t;
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

    struct frame_t
    {
        int number;

        append_vertex_buffer_t *vertexBuffer;
        append_index_buffer_t *indexBuffer;
        command_buffer_t *cmdBuffer;

        vec4_t color;
        bool startedRenderPass;
        Allocator *memory;

        float projectionMatrix[16];
    };

    struct buffer_slice_t
    {
        int offset;
        size_t size;
    };

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