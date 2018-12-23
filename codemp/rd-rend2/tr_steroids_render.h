#pragma once

#include "qcommon/q_math.h"

typedef struct refdef_s refdef_t;

namespace r2
{
namespace impl
{

struct rect_t
{
    int x;
    int y;
    int width;
    int height;
};

struct camera_t
{
    rect_t viewport;
    vec3_t origin;
    vec3_t viewAxis[3];

    float fovx;
    float fovy;

    int viewContents;
    uint32_t renderFlags;
};

void CameraFromRefDef(camera_t *camera, const refdef_t *refdef);

}
}