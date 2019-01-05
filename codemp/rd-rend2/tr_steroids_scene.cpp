#include "tr_steroids_scene.h"

#include "tr_steroids_cmd.h"
#include "tr_steroids_render.h"

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
    namespace
    {
        void GetCulledEntitySurfaces(
            const scene_t *scene,
            const camera_t *camera,
            std::vector<culled_surface_t> &culledSurfaces)
        {
            for (int i = 0; i < scene->entityCount; ++i)
            {
                const entity_t *entity = scene->entities + i;
                const refEntity_t *refEntity = &entity->refEntity;
                switch (refEntity->reType)
                {
                    case RT_MODEL:
                    {
                        const model_t *model = R_GetModelByHandle(refEntity->hModel);
                        switch (model->type)
                        {
                            case MOD_BRUSH:
                                model->data.bmodel;
                                break;

                            case MOD_MDXM:
                            {
                                trRefEntity_t trRefEntity = {};
                                trRefEntity.e = *refEntity;

                                R_AddGhoulSurfaces(
                                    &trRefEntity,
                                    i,
                                    camera,
                                    culledSurfaces);
                                break;
                            }

                            case MOD_MESH:
                                //model->data.mdv;
                                break;

                            default:
                                if (refEntity->ghoul2 != nullptr)
                                {
                                    // UI is dumb and doesn't set the model type
                                    trRefEntity_t trRefEntity = {};
                                    trRefEntity.e = *refEntity;

                                    R_AddGhoulSurfaces(
                                        &trRefEntity,
                                        i,
                                        camera,
                                        culledSurfaces);
                                }

                                ri.Printf(
                                    PRINT_ERROR,
                                    "Invalid model type '%d'",
                                    model->type);
                                break;
                        }
                        break;
                    }

                    case RT_BEAM:
                    {
                        break;
                    }

                    case RT_POLY:
                    {
                        break;
                    }

                    case RT_SPRITE:
                    {
                        break;
                    }

                    case RT_ORIENTED_QUAD:
                    {
                        break;
                    }

                    case RT_SABER_GLOW:
                    {
                        break;
                    }

                    case RT_ELECTRICITY:
                    {
                        break;
                    }

                    case RT_PORTALSURFACE:
                    {
                        break;
                    }

                    case RT_LINE:
                    {
                        break;
                    }

                    case RT_ORIENTEDLINE:
                    {
                        break;
                    }

                    case RT_CYLINDER:
                    {
                        break;
                    }

                    case RT_ENT_CHAIN:
                    {
                        break;
                    }

                    default:
                    {
                        break;
                    }
                }
            }
        }

        std::vector<const UniformData *> MakeEntityUniformData(
            const scene_t *scene,
            Allocator &allocator)
        {
            std::vector<const UniformData *> uniformData(
                static_cast<unsigned>(scene->entityCount));

            UniformDataWriter uniformDataWriter;

            for (int i = 0; i < scene->entityCount; ++i)
            {
                const refEntity_t *ent = &scene->entities[i].refEntity;

                matrix4x4_t modelMatrix = {};
                R_GetModelMatrix(ent, &modelMatrix);

                uniformDataWriter.Start();
                uniformDataWriter.SetUniformMatrix4x4(
                    UNIFORM_MODELMATRIX,
                    modelMatrix.e);

                float volumetricBaseValue = -1.0f;
                if ((ent->renderfx & RF_VOLUMETRIC) != 0)
                {
                    volumetricBaseValue = ent->shaderRGBA[0] / 255.0f;
                }

                uniformDataWriter.SetUniformFloat(
                    UNIFORM_FX_VOLUMETRIC_BASE,
                    volumetricBaseValue);

                uniformData[i] = uniformDataWriter.Finish(allocator);
            }

            return uniformData;
        }

        void GetViewProjectionMatrix(
            const camera_t *camera,
            matrix_t viewProjectionMatrix)
        {
            matrix_t viewMatrix = {};
            viewMatrix[0] = camera->viewAxis[0][0];
            viewMatrix[1] = camera->viewAxis[1][0];
            viewMatrix[2] = camera->viewAxis[2][0];
            viewMatrix[3] = 0.0f;

            viewMatrix[4] = camera->viewAxis[0][1];
            viewMatrix[5] = camera->viewAxis[1][1];
            viewMatrix[6] = camera->viewAxis[2][1];
            viewMatrix[7] = 0.0f;

            viewMatrix[8] = camera->viewAxis[0][2];
            viewMatrix[9] = camera->viewAxis[1][2];
            viewMatrix[10] = camera->viewAxis[2][2];
            viewMatrix[11] = 0.0f;

            viewMatrix[12] = -DotProduct(camera->origin, camera->viewAxis[0]);
            viewMatrix[13] = -DotProduct(camera->origin, camera->viewAxis[1]);
            viewMatrix[14] = -DotProduct(camera->origin, camera->viewAxis[2]);
            viewMatrix[15] = 1.0f;

            matrix_t projectionMatrix = {};
            Matrix16Perspective(
                camera->fovx,
                camera->fovy,
                camera->znear,
                6000.0f,
                projectionMatrix);

            Matrix16Multiply(
                projectionMatrix,
                viewMatrix,
                viewProjectionMatrix);
        }

        const UniformData *MakeSceneCameraUniformData(
            const camera_t *camera,
            Allocator &allocator)
        {
            matrix_t viewProjectionMatrix = {};
            GetViewProjectionMatrix(camera, viewProjectionMatrix);

            UniformDataWriter uniformDataWriter;

            uniformDataWriter.Start();
            uniformDataWriter.SetUniformMatrix4x4(
                UNIFORM_MODELVIEWPROJECTIONMATRIX,
                viewProjectionMatrix);

            return uniformDataWriter.Finish(allocator);
        }
    }

    void SceneClear(scene_t *scene)
    {
        scene->entityCount = 0;
        scene->lightCount = 0;
    }

    bool SceneAddLight(
        scene_t *scene,
        const vec3_t origin,
        float radius,
        bool additive)
    {
        if (scene->lightCount >= MAX_LIGHT_COUNT)
        {
            return false;
        }

        light_t *light = scene->lights + scene->lightCount++;
        VectorCopy(origin, light->origin);
        light->radius = radius;
        light->additive = additive;

        return true;
    }

    bool SceneAddEntity(
        scene_t *scene,
        const refEntity_t *refEntity)
    {
        if (scene->entityCount >= MAX_ENTITY_COUNT)
        {
            return false;
        }

        entity_t *entity = scene->entities + scene->entityCount++;
        entity->refEntity = *refEntity;

        return true;
    }

    void SceneRender(
        frame_t *frame,
        const scene_t *scene,
        const refdef_t *refdef)
    {
        camera_t camera = {};
        CameraFromRefDef(&camera, refdef);

        tr.refdef.rdflags = camera.renderFlags;

        std::vector<culled_surface_t> culledSurfaces;
        GetCulledEntitySurfaces(scene, &camera, culledSurfaces);

        command_buffer_t *cmdBuffer = tr.frame.cmdBuffer;

        // render sun shadow maps
        for (const auto &surface : culledSurfaces)
        {
            // can we batch render the map some how? depth prepass only has two
            // types of shader program

            // draw them using depth prepass shaders
        }

        // depth prepass?
        for (const auto &surface : culledSurfaces)
        {
            // can we batch render the map some how? depth prepass only has two
            // types of shader program

            // draw them using depth prepass shaders
        }

        render_pass_t mainRenderPass = {};
        mainRenderPass.framebuffer = nullptr;
        mainRenderPass.viewport.x = camera.viewport.x;
        mainRenderPass.viewport.y = camera.viewport.y;
        mainRenderPass.viewport.width = camera.viewport.width;
        mainRenderPass.viewport.height = camera.viewport.height;
        mainRenderPass.clearDepthAction = CLEAR_ACTION_FILL;
        mainRenderPass.clearDepth = 1.0f;
        mainRenderPass.clearColorAction[0] = frame->startedRenderPass
                                             ? CLEAR_ACTION_NONE
                                             : CLEAR_ACTION_FILL;
        VectorSet4(mainRenderPass.clearColor[0], 0.0f, 0.0f, 0.0f, 1.0f);

        const std::vector<const UniformData *> entityUniformData =
            MakeEntityUniformData(scene, *tr.frame.memory);

        const UniformData *cameraUniformData = MakeSceneCameraUniformData(
            &camera, *tr.frame.memory);

        // Do main render pass
        UniformDataWriter uniformDataWriter;

        CmdBeginRenderPass(cmdBuffer, &mainRenderPass);
        for (const auto &surface : culledSurfaces)
        {
            // Generate draw items based on q3 shader
            const DrawItem *drawItem = &surface.drawItem;
            const shader_t *shader = surface.shader;

            const shaderStage_t *stage = shader->stages[0];
            const textureBundle_t *textureBundle = &stage->bundle[0];

            render_state_t renderState = {};
            renderState.stateBits = stage->stateBits;
            renderState.cullType = shader->cullType;

            const shaderProgram_t *shaderProgram = stage->glslShaderGroup;
            if (shaderProgram == nullptr)
            {
                shaderProgram = tr.genericShader;
            }
            else
            {
                shaderProgram = shaderProgram + surface.shaderFlags;
            }

            uniformDataWriter.Start();
            uniformDataWriter.SetUniformFloat(
                UNIFORM_FX_VOLUMETRIC_BASE,
                -1.0f);

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

#if 0
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
#endif

            const UniformData *stageUniformData =
                uniformDataWriter.Finish(*tr.frame.memory);

            // draw them
            const UniformData *uniforms[] = {
                stageUniformData,
                drawItem->uniformData,
                entityUniformData[surface.entityNum],
                cameraUniformData};
            CmdSetShaderProgram(
                cmdBuffer,
                shaderProgram,
                ARRAY_LEN(uniforms),
                uniforms);
            CmdSetVertexAttributes(
                cmdBuffer,
                surface.drawItem.numAttributes,
                surface.drawItem.attributes);
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
            CmdSetIndexBuffer(cmdBuffer, drawItem->ibo);
            CmdDrawIndexed(
                cmdBuffer,
                PRIMITIVE_TYPE_TRIANGLES,
                drawItem->draw.params.indexed.numIndices,
                INDEX_TYPE_UINT32,
                drawItem->draw.params.indexed.firstIndex,
                1,
                0);
        }
        CmdEndRenderPass(cmdBuffer);

        frame->sceneRendered = true;
        frame->startedRenderPass = false;
    }
}
