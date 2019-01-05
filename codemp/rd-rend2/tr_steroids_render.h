#pragma once

#include "qcommon/q_math.h"

#include "tr_steroids_math.h"

#include "tr_local.h"

typedef struct refdef_s refdef_t;
typedef struct refEntity_s refEntity_t;
typedef struct shader_s shader_t;
struct UniformData;

namespace r2
{

    struct culled_surface_t
    {
        int entityNum;
        const shader_t *shader;
        uint32_t shaderFlags;
        DrawItem drawItem;
    };

    struct camera_t
    {
        rect_t viewport;
        vec3_t origin;

        matrix_t viewMatrix;
        matrix_t projectionMatrix;
        matrix_t viewProjectionMatrix;
        frustum_t frustumVS; // view space frustum
        frustum_t frustumWS;  // world space frustum

        float znear;
        float fovx;
        float fovy;

        int viewContents;
        uint32_t renderFlags;
    };

    void CameraFromRefDef(camera_t *camera, const refdef_t *refdef);

}