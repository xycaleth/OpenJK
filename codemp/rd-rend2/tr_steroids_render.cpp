#include "tr_steroids_render.h"

#include "qcommon/q_math.h"
#include "rd-common/tr_types.h"
#include "tr_local.h"
#include "tr_steroids_math.h"

namespace r2
{
    namespace
    {
        void FrustumFromTransformMatrix(
            const matrix_t M,
            frustum_t *frustum)
        {
            // Inequalities

            // -w <= x, x <= w
            // -w <= y, y <= w
            // -w <= z, z <= w

            // x = T(p1) * v
            // y = T(p2) * v
            // z = T(p3) * v
            // w = T(p4) * v

            // left -   0 <= x + w = T(p1) * v + T(p4) * v = (T(p1) + T(p4)) * v
            frustum->sides.left.normal[0] = M[ 3] + M[ 0];
            frustum->sides.left.normal[1] = M[ 7] + M[ 4];
            frustum->sides.left.normal[2] = M[11] + M[ 8];
            frustum->sides.left.dist      = M[15] + M[12];
            VectorNormalize(frustum->sides.left.normal);

            // right -  0 <= w - x = T(p4) * v - T(p1) * v = (T(p4) - T(p1)) * v
            frustum->sides.right.normal[0] = M[ 3] - M[ 0];
            frustum->sides.right.normal[1] = M[ 7] - M[ 4];
            frustum->sides.right.normal[2] = M[11] - M[ 8];
            frustum->sides.right.dist      = M[15] - M[12];
            VectorNormalize(frustum->sides.right.normal);

            // bottom - 0 <= y + w = T(p2) * v + T(p4) * v = (T(p2) + T(p4)) * v
            frustum->sides.bottom.normal[0] = M[ 3] + M[ 1];
            frustum->sides.bottom.normal[1] = M[ 7] + M[ 5];
            frustum->sides.bottom.normal[2] = M[11] + M[ 9];
            frustum->sides.bottom.dist      = M[15] + M[13];
            VectorNormalize(frustum->sides.bottom.normal);

            // top -    0 <= w - y = T(p4) * v - T(p2) * v = (T(p4) - T(p2)) * v
            frustum->sides.top.normal[0] = M[ 3] - M[ 1];
            frustum->sides.top.normal[1] = M[ 7] - M[ 5];
            frustum->sides.top.normal[2] = M[11] - M[ 9];
            frustum->sides.top.dist      = M[15] - M[13];
            VectorNormalize(frustum->sides.top.normal);

            // near -   0 <= z + w = T(p3) * v + T(p4) * v = (T(p3) + T(p4)) * v
            frustum->sides.near.normal[0] = M[ 3] + M[ 2];
            frustum->sides.near.normal[1] = M[ 7] + M[ 6];
            frustum->sides.near.normal[2] = M[11] + M[10];
            frustum->sides.near.dist      = M[15] + M[14];
            VectorNormalize(frustum->sides.near.normal);

            // far -    0 <= w - z = T(p4) * v - T(p3) * v = (T(p4) - T(p3)) * v
            frustum->sides.far.normal[0] = M[ 3] - M[ 2];
            frustum->sides.far.normal[1] = M[ 7] - M[ 6];
            frustum->sides.far.normal[2] = M[11] - M[10];
            frustum->sides.far.dist      = M[15] - M[14];
            VectorNormalize(frustum->sides.far.normal);
        }
    }

    void CameraFromRefDef(camera_t *camera, const refdef_t *refdef)
    {
        camera->viewport = {refdef->x, refdef->y, refdef->width,
                            refdef->height};

        VectorCopy(refdef->vieworg, camera->origin);

        camera->viewContents = refdef->viewContents;
        camera->fovx = refdef->fov_x;
        camera->fovy = refdef->fov_y;
        camera->znear = r_znear->value;
        camera->renderFlags = static_cast<uint32_t>(refdef->rdflags);

        vec3_t viewAxis[3];
        AngleVectors(refdef->viewangles, viewAxis[0], viewAxis[1], viewAxis[2]);

        camera->viewMatrix[0] = viewAxis[0][0];
        camera->viewMatrix[1] = viewAxis[1][0];
        camera->viewMatrix[2] = viewAxis[2][0];
        camera->viewMatrix[3] = 0.0f;

        camera->viewMatrix[4] = viewAxis[0][1];
        camera->viewMatrix[5] = viewAxis[1][1];
        camera->viewMatrix[6] = viewAxis[2][1];
        camera->viewMatrix[7] = 0.0f;

        camera->viewMatrix[8] = viewAxis[0][2];
        camera->viewMatrix[9] = viewAxis[1][2];
        camera->viewMatrix[10] = viewAxis[2][2];
        camera->viewMatrix[11] = 0.0f;

        camera->viewMatrix[12] = -DotProduct(camera->origin, viewAxis[0]);
        camera->viewMatrix[13] = -DotProduct(camera->origin, viewAxis[1]);
        camera->viewMatrix[14] = -DotProduct(camera->origin, viewAxis[2]);
        camera->viewMatrix[15] = 1.0f;

        Matrix16Perspective(
            camera->fovx,
            camera->fovy,
            camera->znear,
            6000.0f,
            camera->projectionMatrix);

        Matrix16Multiply(
            camera->projectionMatrix,
            camera->viewMatrix,
            camera->viewProjectionMatrix);

        FrustumFromTransformMatrix(
            camera->projectionMatrix,
            &camera->frustumVS);
        FrustumFromTransformMatrix(
            camera->viewProjectionMatrix,
            &camera->frustumWS);
    }

}