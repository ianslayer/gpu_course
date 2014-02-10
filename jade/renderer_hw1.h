#ifndef RENDERER_HW1_H
#define RENDERER_HW1_H

#include "renderer.h"

namespace jade
{
    void InitRendererHW(class RenderDevice* device, Renderer** renderer);
    void ShutdownRendererHW(Renderer* renderer);
}

#endif
