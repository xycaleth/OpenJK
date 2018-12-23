#include "tr_steroids_scene.h"

#include "tr_steroids_render.h"
#include "tr_local.h"

namespace r2
{

using namespace impl;

void InitScene(scene_t *scene)
{
    Com_Memset(scene, 0, sizeof(*scene));
}

bool AddLight(
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

bool AddEntity(
    scene_t *scene,
    trRefEntity_t *refEntity)
{
    if (scene->entityCount >= MAX_ENTITY_COUNT)
    {
        return false;
    }

    entity_t *entity = scene->entities + scene->entityCount++;
    entity->refEntity = refEntity->e;

    return true;
}

void Render(const scene_t *scene, const refdef_t *refdef)
{
    camera_t camera = {};
    CameraFromRefDef(&camera, refdef);

    for (int i = 0; i < scene->entityCount; ++i)
    {
        const entity_t *entity = scene->entities + i;
        const refEntity_t *refEntity = &entity->refEntity;
        switch (refEntity->reType)
        {
            case RT_MODEL:
            {
                const model_t *model = R_GetModelByHandle(refEntity->hModel);
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

}