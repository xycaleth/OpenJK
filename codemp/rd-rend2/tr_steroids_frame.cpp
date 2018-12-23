#include "tr_steroids_frame.h"

#include "tr_allocator.h"
#include "tr_steroids_backend.h"
#include "tr_steroids_cmd.h"

#include "qgl.h"
#include "tr_local.h"

namespace r2
{

    buffer_slice_t AppendBuffer(
        append_vertex_buffer_t *vertexBuffer,
        const void *data,
        size_t sizeInBytes)
    {
        assert(vertexBuffer != nullptr);
        assert((vertexBuffer->cursor + sizeInBytes) <= vertexBuffer->vbo->vertexesSize);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vbo->vertexesVBO);
        glBufferSubData(
            GL_ARRAY_BUFFER,
            vertexBuffer->cursor,
            sizeInBytes,
            data);

        const buffer_slice_t slice = {vertexBuffer->cursor, sizeInBytes};
        vertexBuffer->cursor += sizeInBytes;
        return slice;
    }

    buffer_slice_t AppendBuffer(
        append_index_buffer_t *indexBuffer,
        const void *data,
        size_t sizeInBytes)
    {
        assert(indexBuffer != nullptr);
        assert((indexBuffer->cursor + sizeInBytes) <= indexBuffer->ibo->indexesSize);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->ibo->indexesVBO);
        glBufferSubData(
            GL_ELEMENT_ARRAY_BUFFER,
            indexBuffer->cursor,
            sizeInBytes,
            data);

        const buffer_slice_t slice = {indexBuffer->cursor, sizeInBytes};
        indexBuffer->cursor += sizeInBytes;
        return slice;
    }

    void InitFrame(frame_t *frame)
    {
        *frame = {};

        size_t totalFrameSize = 0;
        totalFrameSize += sizeof(*frame->cmdBuffer);
        totalFrameSize += sizeof(*frame->vertexBuffer);
        totalFrameSize += sizeof(*frame->indexBuffer);
        totalFrameSize += sizeof(*frame->memory);

        auto *frameMemory = static_cast<byte *>(ri.Hunk_Alloc(
            static_cast<int>(totalFrameSize),
            h_low));

        int offset = 0;
        frame->cmdBuffer = reinterpret_cast<command_buffer_t *>(
            frameMemory + offset);
        offset += sizeof(*frame->cmdBuffer);

        frame->vertexBuffer = reinterpret_cast<append_vertex_buffer_t *>(
            frameMemory + offset);
        offset += sizeof(*frame->vertexBuffer);

        frame->indexBuffer = reinterpret_cast<append_index_buffer_t *>(
            frameMemory + offset);
        offset += sizeof(*frame->indexBuffer);

        frame->memory = new (frameMemory + offset) Allocator(8 * 1024 * 1024);
        offset += sizeof(*frame->memory);

        assert(offset == totalFrameSize);

        ResetCmdBuffer(frame->cmdBuffer);

        VectorSet4(frame->color, 1.0f, 1.0f, 1.0f, 1.0f);

        frame->vertexBuffer->vbo = R_CreateVBO(
            nullptr,
            16 * 1024 * 1024,
            VBO_USAGE_DYNAMIC);

        frame->indexBuffer->ibo = R_CreateIBO(
            nullptr,
            16 * 1024 * 1024,
            VBO_USAGE_DYNAMIC);
    }

    void SubmitFrame(frame_t *frame)
    {
        CmdPresent(frame->cmdBuffer);
        SubmitCommands(frame->cmdBuffer);

        ++frame->number;
        ResetCmdBuffer(frame->cmdBuffer);
        frame->vertexBuffer->cursor = 0;
        frame->indexBuffer->cursor = 0;
        frame->startedRenderPass = false;
        frame->memory->Reset();
    }
}