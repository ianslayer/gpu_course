#ifndef RENDERER_RAY_TRACING_H
#define RENDERER_RAY_TRACING_H
#include "geometry.h"
#include "renderer.h"

namespace jade
{
	struct RTRenderOption
	{
		int width;
		int height;
		int spp;
	};

	void InitRendererRT(class RenderDevice* device, Renderer** renderer);
	void ShutdownRendererRT(Renderer* renderer);
}


#endif
