#include "renderer_hw1.h"
#include "render_device.h"
#include "camera.h"
#include "primitive.h"
#include "material.h"
#include "texture.h"
#include "mesh.h"
#include "scene.h"
#include "light.h"
#include "../matrix.h"
#include "../image.h"

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