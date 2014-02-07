#ifndef RENDERER_GL
#define RENDERER_GL
#include "renderer.h"

namespace jade
{
    class GLRendererOptions
    {
    public:
        GLRendererOptions()
        : dbgDraw(DBG_DRAW_NONE), reloadShaders(false)
        {
            
        }
        enum DebugDraw
        {
            DBG_DRAW_NONE,
            DBG_DRAW_UV_TILING,
            DBG_DRAW_TANGENT_SPACE,
        };
        
        DebugDraw dbgDraw;
        bool      reloadShaders;
    };
    
    void InitRendererGL(class RenderDevice* device, Renderer** renderer);
    void ShutdownRendererGL(Renderer* renderer);
}


#endif