#pragma once

#include "qcommon/q_math.h"

namespace r2
{
    struct rect_t
    {
        int x;
        int y;
        int width;
        int height;
    };

    union frustum_t
    {
        struct
        {
            cplane_t top;
            cplane_t bottom;
            cplane_t left;
            cplane_t right;
            cplane_t front;
            cplane_t back;
        } sides;

        cplane_t planes[6];
    };
}