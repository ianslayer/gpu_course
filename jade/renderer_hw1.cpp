#include "renderer_hw1.h"

namespace jade
{
	class RendererHW : public Renderer
	{
	public:
		RendererHW(RenderDevice* _device) {};
		virtual ~RendererHW() {};
        
		virtual void Render(const Camera* camera, const Scene* scene);
        
	};

    void InitRendererHW(RenderDevice* device, Renderer** renderer)
    {
		*renderer  = new RendererHW(device);
    }
    
    void ShutdownRendererHW(Renderer* renderer)
    {
        delete renderer;
    }
    
    void RendererHW::Render(const Camera* camera, const Scene* scene)
    {
        
    }
    
}