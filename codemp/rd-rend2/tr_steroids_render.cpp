#include "tr_steroids_render.h"

#include "qcommon/q_math.h"
#include "rd-common/tr_types.h"

namespace r2
{
namespace impl
{

void CameraFromRefDef(camera_t *camera, const refdef_t *refdef)
{
    camera->viewport = {refdef->x, refdef->y, refdef->width,
                        refdef->height};
    VectorCopy(refdef->vieworg, camera->origin);
    AngleVectors(
        refdef->viewangles,
        camera->viewAxis[0],
        camera->viewAxis[1],
        camera->viewAxis[2]);
    camera->viewContents = refdef->viewContents;
    camera->fovx = refdef->fov_x;
    camera->fovy = refdef->fov_y;
    camera->renderFlags = static_cast<uint32_t>(refdef->rdflags);
}

}
}