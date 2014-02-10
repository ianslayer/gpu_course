#include "render_device.h"
#include "../window.h"

#ifdef USE_DEVICE_HW1

namespace jade
{
    
VertexBufferHW::VertexBufferHW()
{
    glGenBuffers(1, &vboID);    
}

VertexBufferHW::~VertexBufferHW()
{
    glDeleteBuffers(1, &vboID);    
}

IndexBufferHW::IndexBufferHW()
{
    glGenBuffers(1, &iboID);
}

IndexBufferHW::~IndexBufferHW()
{
    glDeleteBuffers(1, &iboID);    
}

Texture2DHW::Texture2DHW()
{
	glGenTextures(1, &id);
}

Texture2DHW::~Texture2DHW()
{
	glDeleteTextures(1, &id);    
}

TextureSamplerStateHW::TextureSamplerStateHW()
{
   glGenSamplers(1, &sampler );   
}

TextureSamplerStateHW::~TextureSamplerStateHW()
{
    glDeleteSamplers(1, &sampler);    
}

RenderDevice::error_t InitRenderDevice(const Window* window, const RenderDeviceSetting* setting, RenderDevice** device)
{
    *device = new RenderDevice();
    
#ifdef _WIN32
    InitGL(window->hwnd, setting->msaaCount);
#endif
    return RenderDevice::SUCCESS;
}

void ShutdownRenderDevice(RenderDevice* device)
{
    delete device;
}

RenderDevice::error_t RenderDevice::CreateVertexBuffer(size_t size, void* buf, HWVertexBuffer** buffer)
{
    *buffer = new HWVertexBuffer();
   
    return RenderDevice::SUCCESS;
}

RenderDevice::error_t RenderDevice::CreateIndexBuffer(size_t size, void* buf, HWIndexBuffer** buffer)
{
    *buffer = new HWIndexBuffer();
   
    return RenderDevice::SUCCESS;
}

RenderDevice::error_t RenderDevice::CreateTexture2D(HWTexture2D::Desc* desc, SubresourceData* data, HWTexture2D** texture)
{
    *texture = new HWTexture2D();
   
    
    return RenderDevice::SUCCESS;
}


RenderDevice::error_t RenderDevice::CreateSamplerState(TextureSamplerState::Desc* desc, TextureSamplerState** state)
{
    *state = new TextureSamplerState();
    
    return RenderDevice::SUCCESS;
}
    
}

#endif