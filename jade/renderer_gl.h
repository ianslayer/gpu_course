#ifndef RENDERER_GL
#define RENDERER_GL
#include "renderer.h"
#include "../gl_utility.h"

namespace jade
{
    class RendererGL : public Renderer
    {
    public:
        RendererGL();
        virtual ~RendererGL();

        virtual void Render(const Camera* camera, const Scene* scene);
        void RenderWireframe(const Camera* camera, const class Primitive* prim);

        class RenderDevice* device;
        GLuint wireframeShader;
    };

    void InitRendererGL(class RenderDevice* device, RendererGL** renderer);
    void ShutdownRendererGL(RendererGL* renderer);

}


#endif