#pragma once

namespace r2
{
    struct frame_t;

    struct debug_t
    {
        int fontHandle;

        int sceneSubmitMsec;
        int backendMsec;
        int drawCount;

        const frame_t *frame;
    };

    void DebugInit(debug_t *debug, const frame_t *frame);

    void DebugRender(const debug_t *debug);
}