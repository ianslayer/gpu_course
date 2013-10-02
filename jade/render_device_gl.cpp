#include "render_device.h"
#include "../window.h"

namespace jade
{
    VertexBufferGL::VertexBufferGL()
    {
        glGenBuffers(1, &vboID);
    }

    VertexBufferGL::~VertexBufferGL()
    {
        glDeleteBuffers(1, &vboID);
    }

    const VertexBufferImpl* HWVertexBuffer::GetImpl() const
    {
        return &impl;
    }

    VertexBufferImpl* HWVertexBuffer::GetImpl()
    {
        return &impl;
    }

    IndexBufferGL::IndexBufferGL()
    {
        glGenBuffers(1, &iboID);
    }

    IndexBufferGL::~IndexBufferGL()
    {
        glDeleteBuffers(1, &iboID);
    }

    const IndexBufferImpl* HWIndexBuffer::GetImpl() const
    {
        return &impl;
    }

    IndexBufferImpl* HWIndexBuffer::GetImpl()
    {
        return &impl;
    }

    RenderDevice::error_t InitRenderDevice(const Window* window, const RenderDeviceSetting* setting, RenderDevice** device)
    {
        *device = new RenderDevice();
        InitGL(window->hwnd, setting->msaaCount);

        return RenderDevice::SUCCESS;
    }

    void ShutdownRenderDevice(RenderDevice* device)
    {
        delete device;
    }

    RenderDevice::error_t RenderDevice::CreateVertexBuffer(size_t size, void* buf, HWVertexBuffer** buffer)
    {
        *buffer = new HWVertexBuffer();
        (*buffer)->size = size;
        glBindBuffer(GL_ARRAY_BUFFER, (*buffer)->GetImpl()->vboID);
        glBufferData(GL_ARRAY_BUFFER, size, buf, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return RenderDevice::SUCCESS;
    }

    RenderDevice::error_t RenderDevice::CreateIndexBuffer(size_t size, void* buf, HWIndexBuffer** buffer)
    {
        *buffer = new HWIndexBuffer();
        (*buffer)->size = size;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*buffer)->GetImpl()->iboID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buf, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        return RenderDevice::SUCCESS;
    }

}