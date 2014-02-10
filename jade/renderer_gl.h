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
            DBG_DRAW_NONE = 0,
            DBG_DRAW_UV_TILING = 1,
            DBG_DRAW_TANGENT_SPACE = 2,
            DBG_DRAW_DIFFUSE = 3,
            DBG_DRAW_NORMAL = 4,
            DBG_DRAW_SPECULAR = 5,
            
            DBG_DRAW_DIFFUSE_LIGHTING = 6,
            DBG_DRAW_SPECULAR_LIGHTING = 7,
            DBG_DRAW_FRESNEL_SPECULAR_LIGHTING = 8,
            
        };
        
        DebugDraw dbgDraw;
        bool      reloadShaders;
    };
    
    void InitRendererGL(class RenderDevice* device, Renderer** renderer);
    void ShutdownRendererGL(Renderer* renderer);
}


#endif