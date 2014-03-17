#ifndef RENDER_DEVICE_H
#define RENDER_DEVICE_H

#include "refcount.h"

#define USE_DEVICE_GL

#ifdef USE_DEVICE_GL
    #include "render_device_gl.h"
#elif defined (USE_DEVICE_HW1)
    #include "render_device_hw1.h"
#endif

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

    private:
        size_t             size;
        IndexBufferImpl    impl;
    };


	enum TEXTURE_TYPE
	{
		TEX_TYPE_2D,
		TEX_TYPE_3D,
		TEX_TYPE_ARRAY,
	};

	enum TEXTURE_FORMAT
	{
        TEX_FORMAT_R8,
		TEX_FORMAT_RGBA8,
		TEX_FORMAT_SRGB8_ALPHA8,
		TEX_FORMAT_RGBA16F,
		TEX_FORMAT_RGBA32F,

		TEX_FORMAT_DEPTH32F,
	};

	int TotalMipLevels(int width, int height);
	size_t TexelSize(TEXTURE_FORMAT format);
	size_t TextureLevelSize(TEXTURE_FORMAT format, int width, int height, int depth, int mipSlice);
	size_t TextureSize(TEXTURE_FORMAT format, int width, int height, int depth, int mipLevels);
	
	struct SubresourceData
	{
		const void* buf;
	};

	class HWTexture2D : public RefCounted
	{
	public:
		friend class RenderDevice;
		struct Desc
		{
			unsigned int width;
			unsigned int height;
			unsigned int mipLevels;
			unsigned int arraySize;
			TEXTURE_FORMAT format;
			bool generateMipmap;
		};

		size_t				    Size() const;
		const Texture2DImpl*	GetImpl() const;
        const Desc*             GetDesc() const;
    private:
		Desc desc;
		Texture2DImpl impl;
	};
    
    class HWRenderTexture2D : public RefCounted
    {
    public:
        friend class RenderDevice;
        struct Desc
        {
            TEXTURE_FORMAT format;
            unsigned int mipLevel;
        };
        HWTexture2D* GetTexture() const;

		const Desc*					GetDesc() const;
		const RenderTexture2DImpl* GetImpl() const;
		
    private:
        Desc desc;
        RefCountedPtr<HWTexture2D> texture; //referenced original texture;
        RenderTexture2DImpl impl;
    };
    
    class HWDepthStencilSurface : public RefCounted
    {
    public:
        friend class RenderDevice;
        struct Desc
        {
            TEXTURE_FORMAT format;
			unsigned int mipLevel;
        };
        HWTexture2D* GetTexture() const;

		const Desc*					GetDesc() const;
		const DepthStencilSurfaceImpl* GetImpl() const;
    private:
        Desc desc;
        RefCountedPtr<HWTexture2D> texture;
        DepthStencilSurfaceImpl impl;
    };

	class TextureSamplerDesc
	{
	public:
		enum TEX_FILTER
		{
			MIN_MAG_MIP_POINT,
			MIN_MAG_MIP_LINEAR,
			ANISOTROPIC,
		};
	};

	class TextureSamplerState : public RefCounted
	{
	public:
		friend class RenderDevice;

        TextureSamplerState();
        ~TextureSamplerState();
        
		enum TEX_FILTER
		{
			TEX_FILTER_MIN_MAG_MIP_POINT,
			TEX_FILTER_MIN_MAG_MIP_LINEAR,
			TEX_FILTER_ANISOTROPIC,
		};

		enum TEX_ADDRESS_MODE
		{
			TEX_ADDRESS_WRAP,
			TEX_ADDRESS_MIRROR,
			TEX_ADDRESS_CLAMP
		};

		class Desc
		{
		public:

			TEX_FILTER filter;
			TEX_ADDRESS_MODE uAddressMode;
			TEX_ADDRESS_MODE vAddressMode;
			TEX_ADDRESS_MODE wAddressMode;
			unsigned int     maxAnisotropy;
		};

		TextureSamplerStateImpl* GetImpl();
	private:
		Desc desc;
		TextureSamplerStateImpl impl;
	};

    
    struct RenderDeviceSetting
    {
        RenderDeviceSetting()
        {
            msaaCount = 0;
            screenScaleFactor = 1.0f;
        }
        int msaaCount;
        float screenScaleFactor; //support retina display
    };
    
    class RenderDevice
    {
    public:
        enum RDError
        {
            SUCCESS = 0,
            FAIL = 1
        };
        typedef int error_t;

        RenderDevice(const Window* _window) : window(_window) {};
        
        error_t CreateVertexBuffer(size_t size, void* buf, HWVertexBuffer** buffer);
        error_t CreateIndexBuffer(size_t size, void* buf, HWIndexBuffer** buffer);
		error_t CreateTexture2D(HWTexture2D::Desc* desc, SubresourceData* data, HWTexture2D** texture);
		error_t CreateSamplerState(TextureSamplerState::Desc* desc, TextureSamplerState** state);
        error_t CreateRenderTexture2D(HWTexture2D* texture, HWRenderTexture2D::Desc* desc, HWRenderTexture2D** rtTexture);
        error_t CreateRenderTexture2D(HWTexture2D::Desc* texDesc, HWRenderTexture2D::Desc* rtDesc, HWRenderTexture2D** rtTexture);
        error_t CreateDepthStencilSurface(HWTexture2D* texture, HWDepthStencilSurface::Desc* desc,  HWDepthStencilSurface** surface);
        error_t CreateDepthStencilSurface(HWTexture2D::Desc* texDesc, HWDepthStencilSurface::Desc* dsDesc, HWDepthStencilSurface** surface);
        
        RenderDeviceSetting setting;
        const Window*       window;
    };

    RenderDevice::error_t InitRenderDevice(const Window* window, const RenderDeviceSetting* setting, RenderDevice** device);
    void ShutdownRenderDevice(RenderDevice* device);


}

#endif