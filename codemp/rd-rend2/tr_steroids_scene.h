#pragma once

#include "qcommon/q_math.h"
#include "rd-common/tr_types.h"

typedef struct refEntity_s refEntity_t;

namespace r2
{

    struct frame_t;

    struct light_t
    {
        vec3_t origin;
        float radius;
        bool additive;
    };

    struct entity_t
    {
        refEntity_t refEntity;
    };

    const int MAX_LIGHT_COUNT = 128;
    const int MAX_ENTITY_COUNT = 2048;
    struct scene_t
    {
        int lightCount;
        int entityCount;

        light_t lights[MAX_LIGHT_COUNT];
        entity_t entities[MAX_ENTITY_COUNT];
    };

    void SceneClear(scene_t *scene);

    bool SceneAddLight(
        scene_t *scene,
        const vec3_t origin,
        float radius,
        bool additive = true);

    bool SceneAddEntity(
        scene_t *scene,
        const refEntity_t *refEntity);

    void SceneRender(
        frame_t *frame,
        const scene_t *scene,
        const refdef_t *refdef);

}