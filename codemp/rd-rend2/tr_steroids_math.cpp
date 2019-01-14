#include "tr_steroids_math.h"

namespace r2
{
    void NormalizeFrustum(frustum_t *frustum)
    {
        for (int i = 0; i < 6; ++i)
        {
            cplane_t *plane = frustum->planes + i;
            const float length = VectorLength(plane->normal);
            plane->normal[0] /= length;
            plane->normal[1] /= length;
            plane->normal[2] /= length;
            plane->dist /= length;
        }
    }
}