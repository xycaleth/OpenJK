#include "tr_steroids_scene.h"

#include <algorithm>

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
            std::vector<culled_surface_t> &culledSurfaces,
            Allocator &allocator)
        {
            int *particleEntityNums = ojkAllocArray<int>(
                allocator, static_cast<size_t>(scene->entityCount));
            int particleEntitiesCount = 0;

            for (int i = 0; i < scene->entityCount; ++i)
            {
                const entity_t *entity = scene->entities + i;
                const refEntity_t *refEntity = &entity->refEntity;
                switch (refEntity->reType)
                {
                    case RT_BEAM:
                    case RT_POLY:
                    case RT_SPRITE:
                    case RT_ORIENTED_QUAD:
                    case RT_SABER_GLOW:
                    case RT_ELECTRICITY:
                    case RT_LINE:
                    case RT_ORIENTEDLINE:
                    case RT_CYLINDER:
                    case RT_ENT_CHAIN:
                        particleEntityNums[particleEntitiesCount++] = i;
                        break;

                    default:
                        break;
                }
            }

            const entity_t *entities = scene->entities;
            std::sort(
                particleEntityNums,
                particleEntityNums + particleEntitiesCount,
                [entities](int a, int b)
                {
                    return entities[a].refEntity.customShader <
                        entities[b].refEntity.customShader;
                });

            for (int i = 0; i < scene->entityCount; ++i)
            {
                const entity_t *entity = scene->entities + i;
                const refEntity_t *refEntity = &entity->refEntity;
                switch (refEntity->reType)
                {
                    case RT_MODEL:
                    {
                        const model_t *model = R_GetModelByHandle(refEntity->hModel);
                        trRefEntity_t trRefEntity = {};
                        trRefEntity.e = *refEntity;

                        switch (model->type)
                        {
                            case MOD_BRUSH:
                            {
                                R_AddBrushModelSurfaces(
                                    &trRefEntity,
                                    i,
                                    camera,
                                    culledSurfaces);
                                break;
                            }

                            case MOD_MDXM:
                            {
                                R_AddGhoulSurfaces(
                                    &trRefEntity,
                                    i,
                                    camera,
                                    culledSurfaces);
                                break;
                            }

                            case MOD_MESH:
                            {
                                R_AddMD3Surfaces(
                                    &trRefEntity,
                                    i,
                                    camera,
                                    culledSurfaces);
                                break;
                            }

                            default:
                            {
                                if (refEntity->ghoul2 != nullptr)
                                {
                                    // UI is dumb and doesn't set the model type
                                    R_AddGhoulSurfaces(
                                        &trRefEntity,
                                        i,
                                        camera,
                                        culledSurfaces);
                                }
                                else
                                {
                                    ri.Printf(
                                        PRINT_ERROR,
                                        "Invalid model type '%d'",
                                        model->type);
                                }
                                break;
                            }
                        }
                        break;
                    }

                    default:
                        break;
                }
            }
        }

        std::vector<const UniformData *> MakeEntityUniformData(
            const scene_t *scene,
            UniformDataWriter &uniformDataWriter,
            Allocator &allocator)
        {
            std::vector<const UniformData *> uniformData(
                static_cast<unsigned>(scene->entityCount));

            for (int i = 0; i < scene->entityCount; ++i)
            {
                const refEntity_t *ent = &scene->entities[i].refEntity;
                vec3_t entOrigin;
                vec3_t lightDir;
                vec3_t directionalLight;
                vec3_t ambientLight;

                VectorCopy(ent->origin, entOrigin);

                matrix4x4_t modelMatrix = {};
                R_GetModelMatrix(ent, &modelMatrix);

                uniformDataWriter.Start();
                uniformDataWriter.SetUniformMatrix4x4(
                    UNIFORM_MODELMATRIX,
                    modelMatrix.e);

                if (R_LightForPoint(
                        entOrigin,
                        ambientLight,
                        directionalLight,
                        lightDir))
                {
                    VectorScale(ambientLight, 1.0f / 255.0f, ambientLight);
                    VectorScale(directionalLight, 1.0f / 255.0f, directionalLight);
                    uniformDataWriter.SetUniformVec3(
                        UNIFORM_AMBIENTLIGHT, ambientLight);
                    uniformDataWriter.SetUniformVec3(
                        UNIFORM_DIRECTEDLIGHT, directionalLight);
                    uniformDataWriter.SetUniformVec3(
                        UNIFORM_MODELLIGHTDIR, lightDir);
                }

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

        const UniformData *MakeWorldEntityUniformData(
            const scene_t *scene,
            UniformDataWriter &uniformDataWriter,
            Allocator &allocator)
        {
            matrix4x4_t modelMatrix = {};
            modelMatrix.e[0] = 1.0f;
            modelMatrix.e[5] = 1.0f;
            modelMatrix.e[10] = 1.0f;
            modelMatrix.e[15] = 1.0f;

            uniformDataWriter.Start();
            uniformDataWriter.SetUniformMatrix4x4(
                UNIFORM_MODELMATRIX,
                modelMatrix.e);

            uniformDataWriter.SetUniformFloat(
                UNIFORM_FX_VOLUMETRIC_BASE,
                -1.0f);

            return uniformDataWriter.Finish(allocator);
        }

        const UniformData *MakeSceneCameraUniformData(
            const camera_t *camera,
            UniformDataWriter &uniformDataWriter,
            Allocator &allocator)
        {
            uniformDataWriter.Start();
            uniformDataWriter.SetUniformMatrix4x4(
                UNIFORM_MODELVIEWPROJECTIONMATRIX,
                camera->viewProjectionMatrix);

            return uniformDataWriter.Finish(allocator);
        }

        const UniformData *MakeStageUniformData(
            const shaderStage_t *stage,
            UniformDataWriter &uniformDataWriter,
            Allocator &allocator)
        {
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

            return uniformDataWriter.Finish(allocator);
        }

        void SceneCmdSetTextureFromBundle(
            command_buffer_t *cmdBuffer,
            const textureBundle_t *textureBundle,
            int index)
        {
            if (textureBundle->isVideoMap)
            {
                ri.CIN_RunCinematic(textureBundle->videoMapHandle);
                ri.CIN_UploadCinematic(textureBundle->videoMapHandle);

                CmdSetTexture(
                    cmdBuffer,
                    index,
                    tr.scratchImage[textureBundle->videoMapHandle]);
            }
            else
            {
                CmdSetTexture(
                    cmdBuffer,
                    index,
                    textureBundle->image[0]);
            }
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
        const int startTime = ri.Milliseconds();

        camera_t camera = {};
        CameraFromRefDef(&camera, refdef);

        // FIXME: SetupEntityLighting checks refdef.rdflags. Need to fix that
        // at some point
        tr.refdef.rdflags = camera.renderFlags;
        tr.refdef.areamaskModified = qtrue;
        Com_Memcpy(
            tr.refdef.areamask,
            refdef->areamask,
            sizeof(refdef->areamask));

        std::vector<culled_surface_t> culledSurfaces;
        GetCulledEntitySurfaces(scene, &camera, culledSurfaces, *frame->memory);
        tr.debug.entitySurfaceCount = culledSurfaces.size();

        R_AddWorldSurfaces(&camera, culledSurfaces);
        tr.debug.worldSurfaceCount =
            culledSurfaces.size() - tr.debug.entitySurfaceCount;

        std::sort(
            std::begin(culledSurfaces),
            std::end(culledSurfaces),
            [](const culled_surface_t &a, const culled_surface_t &b)
            {
                if (a.shader->sortedIndex < b.shader->sortedIndex)
                {
                    return true;
                }
                else if (a.shader->sortedIndex > b.shader->sortedIndex)
                {
                    return false;
                }

                return false;
            });

        command_buffer_t *cmdBuffer = frame->cmdBuffer;

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

        UniformDataWriter uniformDataWriter;

        const UniformData *worldEntityUniformData =
            MakeWorldEntityUniformData(
                scene,
                uniformDataWriter,
                *frame->memory);

        const std::vector<const UniformData *> entityUniformData =
            MakeEntityUniformData(scene, uniformDataWriter, *frame->memory);

        const UniformData *cameraUniformData = MakeSceneCameraUniformData(
            &camera, uniformDataWriter, *frame->memory);

        vec4_t sunDirection = {0.0f, 0.0f, 0.0f, 0.0f};
        VectorNormalize2(tr.sunDirection, sunDirection);

        vec3_t sunColor;
        VectorCopy(tr.sunLight, sunColor);

        uniformDataWriter.Start();
        uniformDataWriter.SetUniformVec4(
            UNIFORM_PRIMARYLIGHTORIGIN,
            sunDirection);
        uniformDataWriter.SetUniformVec3(UNIFORM_PRIMARYLIGHTCOLOR, sunColor);
        const UniformData *sceneUniformData =
            uniformDataWriter.Finish(*frame->memory);

        {
            render_pass_t mainRenderPass = {};
            mainRenderPass.framebuffer = tr.renderFbo;
            mainRenderPass.viewport = camera.viewport;
            mainRenderPass.clearDepthAction = CLEAR_ACTION_FILL;
            mainRenderPass.clearDepth = 1.0f;
            mainRenderPass.clearColorAction[0] = CLEAR_ACTION_FILL;
            VectorSet4(mainRenderPass.clearColor[0], 0.0f, 0.0f, 0.0f, 0.0f);

            CmdBeginRenderPass(cmdBuffer, &mainRenderPass);
            for (const auto &surface : culledSurfaces)
            {
                // Generate draw items based on q3 shader
                const DrawItem *drawItem = &surface.drawItem;
                const shader_t *shader = surface.shader;

                const shaderStage_t *stage = shader->stages[0];
                if (stage == nullptr)
                {
                    // FIX ME: Probably sky. Deal with this
                    continue;
                }

                render_state_t renderState = {};
                renderState.stateBits = stage->stateBits;
                renderState.cullType = shader->cullType;

                const UniformData *stageUniformData = MakeStageUniformData(
                    stage, uniformDataWriter, *frame->memory);

                // draw them
                const UniformData *uniforms[] = {
                    stageUniformData,
                    drawItem->uniformData,
                    surface.entityNum == REFENTITYNUM_WORLD
                        ? worldEntityUniformData
                        : entityUniformData[surface.entityNum],
                    cameraUniformData,
                    sceneUniformData};

                uint32_t shaderVariant = surface.shaderFlags;

                if (stage->bundle[TB_DIFFUSEMAP].image[0] != nullptr)
                {
                    SceneCmdSetTextureFromBundle(
                        cmdBuffer,
                        &stage->bundle[TB_DIFFUSEMAP],
                        TB_DIFFUSEMAP);
                }

                if (stage->bundle[TB_LIGHTMAP].image[0] != nullptr)
                {
                    SceneCmdSetTextureFromBundle(
                        cmdBuffer,
                        &stage->bundle[TB_LIGHTMAP],
                        TB_LIGHTMAP);
                    shaderVariant |= LIGHTDEF_USE_LIGHTMAP;
                }

                const shaderProgram_t *shaderProgram = stage->glslShaderGroup;
                if (shaderProgram == nullptr)
                {
                    shaderProgram = tr.genericShader;
                }
                else
                {
                    shaderProgram = shaderProgram + shaderVariant;
                }

                CmdSetShaderProgram(
                    cmdBuffer,
                    shaderProgram,
                    ARRAY_LEN(uniforms),
                    uniforms);
                CmdSetVertexAttributes(
                    cmdBuffer,
                    surface.drawItem.numAttributes,
                    surface.drawItem.attributes);
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
        }

        {
            render_pass_t toneMapPass = {};
            toneMapPass.framebuffer = nullptr;
            toneMapPass.viewport = camera.viewport;
            toneMapPass.clearDepthAction = CLEAR_ACTION_NONE;
            VectorSet4(toneMapPass.clearColor[0], 0.0f, 0.0f, 0.0f, 1.0f);
            toneMapPass.clearColorAction[0] = frame->startedRenderPass
                                              ? CLEAR_ACTION_NONE
                                              : CLEAR_ACTION_FILL;

            render_state_t renderState = {};
            renderState.stateBits = GLS_DEPTHTEST_DISABLE;
            renderState.cullType = CT_TWO_SIDED;

            CmdBeginRenderPass(cmdBuffer, &toneMapPass);
            CmdSetShaderProgram(cmdBuffer, &tr.tonemapShader);
            CmdSetRenderState(cmdBuffer, &renderState);
            CmdSetTexture(cmdBuffer, 0, tr.renderImage);
            CmdDraw(cmdBuffer, PRIMITIVE_TYPE_TRIANGLES, 3, 0, 1);
            CmdEndRenderPass(cmdBuffer);
        }

        frame->sceneRendered = true;
        frame->startedRenderPass = false;

        const int endTime = ri.Milliseconds();
        tr.debug.sceneSubmitMsec = endTime - startTime;
        tr.debug.entityCount = scene->entityCount;

        ++tr.viewCount;
    }
}
