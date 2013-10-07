#ifndef RENDERER_GL
#define RENDERER_GL
#include "renderer.h"

namespace jade
{
    void InitRendererGL(class RenderDevice* device, Renderer** renderer);
    void ShutdownRendererGL(Renderer* renderer);
}


#endif