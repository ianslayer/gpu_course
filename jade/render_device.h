#ifndef RENDER_DEVICE_H
#define RENDER_DEVICE_H

#include "refcount.h"
#include "render_device_gl.h"

struct Window;

namespace jade
{
    class HWVertexBuffer : public RefCounted
    {
    friend class RenderDevice;
    public:
        HWVertexBuffer();
        ~HWVertexBuffer();

        size_t				    Size() const;
        const VertexBufferImpl*	GetImpl() const;
        VertexBufferImpl*       GetImpl();

    private:
        size_t              size;
        VertexBufferImpl    impl;
    };

    class HWIndexBuffer : public RefCounted
    {
    friend class RenderDevice;
    public:
        HWIndexBuffer();
        ~HWIndexBuffer();

        size_t				    Size() const;
        size_t                  IndexCount() const;
        const IndexBufferImpl*	GetImpl() const;
        IndexBufferImpl*        GetImpl();

    private:
        size_t             size;
        IndexBufferImpl    impl;
    };

    class HWTexture : public RefCounted
    {
    friend class RenderDevice;
    public:

    private:

    };

    class RenderDevice
    {
    public:
        enum RDError
        {
            SUCCESS = 0
        };
        typedef int error_t;

        error_t CreateVertexBuffer(size_t size, void* buf, HWVertexBuffer** buffer);
        error_t CreateIndexBuffer(size_t size, void* buf, HWIndexBuffer** buffer);

    };

    struct RenderDeviceSetting
    {
        RenderDeviceSetting()
        {
            msaaCount = 0;
        }
        int msaaCount;
    };

    RenderDevice::error_t InitRenderDevice(const Window* window, const RenderDeviceSetting* setting, RenderDevice** device);
    void ShutdownRenderDevice(RenderDevice* device);


}

#endif