#include "tr_steroids_frame.h"

#include "tr_allocator.h"
#include "tr_steroids_backend.h"
#include "tr_steroids_cmd.h"
#include "tr_steroids_debug.h"

#include "qgl.h"
#include "tr_local.h"

void ComputeTexMods(
    const shaderStage_t *pStage,
    int bundleNum,
    float time,
    float *outMatrix,
    float *outOffTurb);

void ComputeShaderColors(
    const shaderStage_t *pStage,
    vec4_t baseColor,
    vec4_t vertColor,
    int blend,
    colorGen_t *forceRGBGen,
    alphaGen_t *forceAlphaGen);

namespace r2
{

    buffer_slice_t AppendBuffer(
        append_vertex_buffer_t *vertexBuffer,
        const void *data,
        size_t sizeInBytes)
    {
        assert(vertexBuffer != nullptr);
        assert((vertexBuffer->cursor + sizeInBytes) <=
                   vertexBuffer->vbo->vertexesSize);

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
        assert((indexBuffer->cursor + sizeInBytes) <=
                   indexBuffer->ibo->indexesSize);

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

    const UniformData *GetUniformDataForStage(
        const shaderStage_t *stage,
        int shaderVariant,
        const float timeInSeconds)
    {
        UniformDataWriter uniformDataWriter;
        uniformDataWriter.Start();

        vec4_t vertColor;
        vec4_t baseColor;
        colorGen_t rgbGen = CGEN_BAD;
        alphaGen_t alphaGen = AGEN_IDENTITY;

        ComputeShaderColors(
            stage,
            baseColor,
            vertColor,
            stage->stateBits,
            &rgbGen,
            &alphaGen);

        uniformDataWriter.SetUniformVec4(UNIFORM_VERTCOLOR, vertColor);
        uniformDataWriter.SetUniformVec4(UNIFORM_BASECOLOR, baseColor);
        uniformDataWriter.SetUniformInt(UNIFORM_COLORGEN, rgbGen);
        uniformDataWriter.SetUniformInt(UNIFORM_ALPHAGEN, alphaGen);

        if ((shaderVariant & GENERICDEF_USE_TCGEN_AND_TCMOD) != 0)
        {
            float matrix[4];
            float turbulence[4];
            ComputeTexMods(stage, 0, timeInSeconds, matrix, turbulence);

            uniformDataWriter.SetUniformVec4(
                UNIFORM_DIFFUSETEXMATRIX,
                matrix);
            uniformDataWriter.SetUniformVec4(
                UNIFORM_DIFFUSETEXOFFTURB,
                turbulence);

            uniformDataWriter.SetUniformInt(
                UNIFORM_TCGEN0,
                stage->bundle[0].tcGen);
            uniformDataWriter.SetUniformInt(
                UNIFORM_TCGEN1,
                stage->bundle[1].tcGen);

            if (stage->bundle[0].tcGen == TCGEN_VECTOR)
            {
                uniformDataWriter.SetUniformVec3(
                    UNIFORM_TCGEN0VECTOR0,
                    stage->bundle[0].tcGenVectors[0]);
                uniformDataWriter.SetUniformVec3(
                    UNIFORM_TCGEN0VECTOR1,
                    stage->bundle[0].tcGenVectors[1]);
            }
        }

        return uniformDataWriter.Finish(*tr.frame.memory);
    }

    const UniformData *GetFrameUniformData()
    {
        matrix_t projectionMatrix;
        Matrix16Ortho(0.0f, 640.0f, 480.0f, 0.0f, 0.0f, 1.0f, projectionMatrix);

        UniformDataWriter uniformDataWriter;
        uniformDataWriter.Start();
        uniformDataWriter.SetUniformMatrix4x4(
            UNIFORM_MODELVIEWPROJECTIONMATRIX,
            projectionMatrix);
        uniformDataWriter.SetUniformFloat(
            UNIFORM_FX_VOLUMETRIC_BASE,
            -1.0f);
        return uniformDataWriter.Finish(*tr.frame.memory);
    }

    void FlushQuadsBatch(draw_batch_t *drawBatch)
    {
        if (drawBatch->indexData.empty())
        {
            return;
        }

        const auto vertexBufferSlice =
            AppendBuffer(
                tr.frame.vertexBuffer,
                drawBatch->vertexData.data(),
                drawBatch->vertexData.size());
        const auto indexBufferSlice =
            AppendBuffer(
                tr.frame.indexBuffer,
                drawBatch->indexData.data(),
                drawBatch->indexData.size());
        const shader_t *shader = tr.frame.batchShader;

        vertexAttribute_t vertexAttributes[3] = {};
        vertexAttributes[0].vbo = tr.frame.vertexBuffer->vbo;
        vertexAttributes[0].index = ATTR_INDEX_POSITION;
        vertexAttributes[0].numComponents = 2;
        vertexAttributes[0].integerAttribute = GL_FALSE;
        vertexAttributes[0].type = GL_FLOAT;
        vertexAttributes[0].normalize = GL_FALSE;
        vertexAttributes[0].stride = sizeof(quad_vertex_t);
        vertexAttributes[0].offset = static_cast<int>(
            vertexBufferSlice.offset + offsetof(quad_vertex_t, position));
        vertexAttributes[0].stepRate = 0;

        vertexAttributes[1].vbo = tr.frame.vertexBuffer->vbo;
        vertexAttributes[1].index = ATTR_INDEX_TEXCOORD0;
        vertexAttributes[1].numComponents = 2;
        vertexAttributes[1].integerAttribute = GL_FALSE;
        vertexAttributes[1].type = GL_FLOAT;
        vertexAttributes[1].normalize = GL_FALSE;
        vertexAttributes[1].stride = sizeof(quad_vertex_t);
        vertexAttributes[1].offset = static_cast<int>(
            vertexBufferSlice.offset + offsetof(quad_vertex_t, texcoord));
        vertexAttributes[1].stepRate = 0;

        vertexAttributes[2].vbo = tr.frame.vertexBuffer->vbo;
        vertexAttributes[2].index = ATTR_INDEX_COLOR;
        vertexAttributes[2].numComponents = 4;
        vertexAttributes[2].integerAttribute = GL_FALSE;
        vertexAttributes[2].type = GL_FLOAT;
        vertexAttributes[2].normalize = GL_FALSE;
        vertexAttributes[2].stride = sizeof(quad_vertex_t);
        vertexAttributes[2].offset = static_cast<int>(
            vertexBufferSlice.offset + offsetof(quad_vertex_t, color));
        vertexAttributes[2].stepRate = 0;

        command_buffer_t *cmdBuffer = tr.frame.cmdBuffer;

        if (!tr.frame.startedRenderPass)
        {
            render_pass_t renderPass = {};
            renderPass.viewport = {0, 0, glConfig.vidWidth, glConfig.vidHeight};
            renderPass.clearColorAction[0] = tr.frame.sceneRendered
                                             ? CLEAR_ACTION_NONE
                                             : CLEAR_ACTION_FILL;
            VectorSet4(renderPass.clearColor[0], 0.0f, 0.0f, 0.0f, 1.0f);
            renderPass.clearDepthAction = CLEAR_ACTION_FILL;
            renderPass.clearDepth = 1.0f;

            CmdBeginRenderPass(cmdBuffer, &renderPass);

            tr.frame.startedRenderPass = true;
        }

        const float timeInSeconds = 0.001f * ri.Milliseconds();
        const UniformData *frameUniformData = GetFrameUniformData();

        for (int i = 0; i < shader->numUnfoggedPasses; ++i)
        {
            const shaderStage_t *stage = shader->stages[i];
            if (stage == nullptr)
            {
                continue;
            }

            render_state_t renderState = {};
            renderState.stateBits = stage->stateBits;
            renderState.cullType = CT_TWO_SIDED;

            const textureBundle_t *textureBundle = &stage->bundle[0];
            int shaderVariant = 0;
            if (textureBundle->tcGen != TCGEN_TEXTURE ||
                textureBundle->numTexMods > 0)
            {
                shaderVariant |= GENERICDEF_USE_TCGEN_AND_TCMOD;
            }

            const shaderProgram_t *shaderProgram =
                tr.genericShader + shaderVariant;
            const UniformData *uniformData = GetUniformDataForStage(
                stage,
                shaderVariant,
                timeInSeconds);

            const UniformData *uniforms[] = {frameUniformData, uniformData};
            CmdSetShaderProgram(cmdBuffer, shaderProgram, 2, uniforms);
            CmdSetVertexAttributes(cmdBuffer, 3, vertexAttributes);
            if (textureBundle->isVideoMap)
            {
                ri.CIN_RunCinematic(textureBundle->videoMapHandle);
                ri.CIN_UploadCinematic(textureBundle->videoMapHandle);

                CmdSetTexture(
                    cmdBuffer,
                    0,
                    tr.scratchImage[textureBundle->videoMapHandle]);
            }
            else
            {
                CmdSetTexture(
                    cmdBuffer,
                    0,
                    textureBundle->image[0]);
            }

            CmdSetRenderState(cmdBuffer, &renderState);
            CmdSetIndexBuffer(cmdBuffer, tr.frame.indexBuffer->ibo);
            CmdDrawIndexed(
                cmdBuffer,
                PRIMITIVE_TYPE_TRIANGLES,
                drawBatch->indexCount,
                INDEX_TYPE_UINT16,
                indexBufferSlice.offset,
                1,
                0);
        }

        ResetDrawBatch(drawBatch);
    }
    
    void ResetDrawBatch(draw_batch_t *drawBatch)
    {
        drawBatch->vertexData.clear();
        drawBatch->vertexCount = 0;

        drawBatch->indexData.clear();
        drawBatch->indexCount = 0;
    }

    void AppendDrawBatch(
        draw_batch_t *drawBatch,
        const void *vertexData,
        size_t vertexDataSize,
        int vertexCount,
        const void *indexData,
        size_t indexDataSize,
        int indexCount)
    {
        drawBatch->vertexData.insert(
            drawBatch->vertexData.end(),
            static_cast<const byte *>(vertexData),
            static_cast<const byte *>(vertexData) + vertexDataSize);

        drawBatch->indexData.insert(
            drawBatch->indexData.end(),
            static_cast<const byte *>(indexData),
            static_cast<const byte *>(indexData) + indexDataSize);

        drawBatch->vertexCount += vertexCount;
        drawBatch->indexCount += indexCount;
    }

    void InitFrame(frame_t *frame)
    {
        *frame = {};

        // Allocate all the memory we need
        size_t totalFrameSize = 0;
        totalFrameSize += sizeof(*frame->cmdBuffer);
        totalFrameSize += sizeof(*frame->vertexBuffer);
        totalFrameSize += sizeof(*frame->indexBuffer);
        totalFrameSize += sizeof(*frame->memory);

        // Assign the pointers
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

        // Initialize variables
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
        
        ResetDrawBatch(&frame->quadsBatch);
        frame->batchShader = nullptr;
    }

    void SubmitFrame(frame_t *frame)
    {
        if (r_speeds->integer > 9000)
        {
            DebugRender(&tr.debug);
        }

        FlushQuadsBatch(&frame->quadsBatch);
        frame->batchShader = nullptr;
        
        CmdPresent(frame->cmdBuffer);
        SubmitCommands(frame->cmdBuffer);

        ++frame->number;
        ResetCmdBuffer(frame->cmdBuffer);
        frame->vertexBuffer->cursor = 0;
        frame->indexBuffer->cursor = 0;
        frame->sceneRendered = false;
        frame->startedRenderPass = false;
        frame->memory->Reset();
    }

    void FreeFrame(frame_t *frame)
    {
        if (frame->memory != nullptr)
        {
            frame->memory->~Allocator();
            frame->memory = nullptr;
        }
    }
}