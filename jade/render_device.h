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


	enum TEXTURE_TYPE
	{
		TEX_TYPE_2D,
		TEX_TYPE_3D,
		TEX_TYPE_ARRAY,
	};

	enum TEXTURE_FORMAT
	{
		TEX_FORMAT_RGBA8,
		TEX_FORMAT_SRGB8_ALPHA8,
		TEX_FORMAT_RGBA16F,
		TEX_FORMAT_RGBA32F,
	};

	int TotalMipLevels(int width, int height);
	size_t TexelSize(TEXTURE_FORMAT format);
	size_t TextureLevelSize(TEXTURE_FORMAT format, int width, int height, int depth, int mipSlice);
	size_t TextureSize(TEXTURE_FORMAT format, int width, int height, int depth, int mipLevels);
	
	struct SubresourceData
	{
		const void* buf;
		//unsigned int pitch;
		//unsigned int slicePitch;
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
		Texture2DImpl*			GetImpl();
    private:
		Desc desc;
		Texture2DImpl impl;
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
		error_t CreateTexture2D(HWTexture2D::Desc* desc, SubresourceData* data, HWTexture2D** texture);
		error_t CreateSamplerState(TextureSamplerState::Desc* desc, TextureSamplerState** state);
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