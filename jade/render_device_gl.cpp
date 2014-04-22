#include "render_device.h"
#include "../window.h"

#ifdef USE_DEVICE_GL
namespace jade
{
	static GLenum GetGLSizedTexFormat(TEXTURE_FORMAT format)
	{
		switch(format)
		{
        case TEX_FORMAT_R8:
            return GL_R8;
		case TEX_FORMAT_RGBA8:
			return GL_RGBA8;
		case TEX_FORMAT_RGBA16F:
			return GL_RGBA16F;
        case TEX_FORMAT_RGBA32F:
            return GL_RGBA32F;
		case TEX_FORMAT_R32F:
			return GL_R32F;
		case TEX_FORMAT_RG32F:
			return GL_RG32F;
		case TEX_FORMAT_SRGB8_ALPHA8:
			return GL_SRGB8_ALPHA8;
		case TEX_FORMAT_DEPTH32F:
			return GL_DEPTH_COMPONENT32F;
		}

		return 0;
	}

	static GLenum GetGLTexFormat(TEXTURE_FORMAT format)
	{
		switch(format)
		{
        case TEX_FORMAT_R8:
            return GL_RED;
		case TEX_FORMAT_RGBA8:
			return GL_RGBA;
		case TEX_FORMAT_RGBA16F:
			return GL_RGBA;
        case TEX_FORMAT_RGBA32F:
            return GL_RGBA;
		case TEX_FORMAT_R32F:
			return GL_RED;
		case TEX_FORMAT_RG32F:
			return GL_RG;
		case TEX_FORMAT_SRGB8_ALPHA8:
			return GL_RGBA;
		case TEX_FORMAT_DEPTH32F:
			return GL_DEPTH_COMPONENT;			
		}
        return 0;
	}

	static GLenum GetGLTexDataType(TEXTURE_FORMAT format)
	{
		switch(format)
		{
        case TEX_FORMAT_R8:
            return GL_UNSIGNED_BYTE;
		case TEX_FORMAT_RGBA8:
			return GL_UNSIGNED_BYTE;
		case TEX_FORMAT_RGBA16F:
			return GL_HALF_FLOAT;
        case TEX_FORMAT_RGBA32F:
            return GL_FLOAT;
		case TEX_FORMAT_R32F:
			return GL_FLOAT;
		case TEX_FORMAT_RG32F:
			return GL_FLOAT;
		case TEX_FORMAT_SRGB8_ALPHA8:
			return GL_UNSIGNED_BYTE;
		case TEX_FORMAT_DEPTH32F:
			return GL_FLOAT;
		}

		return 0;

	}

    VertexBufferGL::VertexBufferGL()
    {
        glGenBuffers(1, &vboID);
    }

    VertexBufferGL::~VertexBufferGL()
    {
        glDeleteBuffers(1, &vboID);
    }

    IndexBufferGL::IndexBufferGL()
    {
        glGenBuffers(1, &iboID);
    }

    IndexBufferGL::~IndexBufferGL()
    {
        glDeleteBuffers(1, &iboID);
    }

	Texture2DGL::Texture2DGL()
	{
		glGenTextures(1, &id);
	}

	Texture2DGL::~Texture2DGL()
	{
		glDeleteTextures(1, &id);
	}
    
    TextureSamplerStateGL::TextureSamplerStateGL()
    {
        glGenSamplers(1, &sampler );
    }

    TextureSamplerStateGL::~TextureSamplerStateGL()
    {
        glDeleteSamplers(1, &sampler);
    }
    
    RenderDevice::error_t InitRenderDevice(const Window* window, const RenderDeviceSetting* setting, RenderDevice** device)
    {
#ifdef _WIN32
        InitGL(window->hwnd, setting->msaaCount);
#endif
        
        *device = new RenderDevice(window);
        (*device)->setting = *setting;
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

	RenderDevice::error_t RenderDevice::CreateTexture2D(HWTexture2D::Desc* desc, SubresourceData* data, HWTexture2D** texture)
	{
		*texture = new HWTexture2D();
		glBindTexture(GL_TEXTURE_2D, (*texture)->GetImpl()->id);

		/*
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, desc->width, desc->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data->buf);
		if(desc->generateMipmap)
			glGenerateMipmap(GL_TEXTURE_2D);
		*/	
		
		
		glTexStorage2D(GL_TEXTURE_2D, desc->mipLevels, GetGLSizedTexFormat(desc->format), desc->width, desc->height);
		
		if(data )
		{
            int mipLevel = 0;
            int mipWidth = desc->width;
            int mipHeight = desc->height;
            
            do {
                glTexSubImage2D(GL_TEXTURE_2D,
                                mipLevel,
                                0, 0,
                                mipWidth, mipHeight,
                                GetGLTexFormat(desc->format),
                                GetGLTexDataType(desc->format),
                                data[mipLevel].buf
                );
                
                mipLevel++;
                mipWidth /=2;
                mipHeight /=2;
            } while (mipLevel < desc->mipLevels && !desc->generateMipmap);

			if(desc->generateMipmap)
				glGenerateMipmap(GL_TEXTURE_2D);
		}
		

		(*texture)->desc = *desc;

		return RenderDevice::SUCCESS;
	}

    RenderDevice::error_t RenderDevice::CreateRenderTexture2D(HWTexture2D *texture, HWRenderTexture2D::Desc *desc, HWRenderTexture2D **rtTexture)
    {
		if(texture == NULL)
			return RenderDevice::FAIL;

        *rtTexture = new HWRenderTexture2D();
		(*rtTexture)->texture = texture;
		(*rtTexture)->desc = *desc;
        if(texture->GetDesc()->format != desc->format) //TODO: create another texture view
        {
            
        }
       
        return RenderDevice::SUCCESS;
    }
    
    RenderDevice::error_t RenderDevice::CreateRenderTexture2D(HWTexture2D::Desc* texDesc, HWRenderTexture2D::Desc* rtDesc, HWRenderTexture2D** rtTexture)
    {
        HWTexture2D* texture = NULL;
        error_t err = CreateTexture2D(texDesc, NULL, &texture);
        if(err!= SUCCESS)
            return err;
   
        err = CreateRenderTexture2D(texture, rtDesc, rtTexture);
        
        return err;
    }
    
	RenderDevice::error_t RenderDevice::CreateDepthStencilSurface(HWTexture2D* texture, HWDepthStencilSurface::Desc* desc, HWDepthStencilSurface** surface)
	{
		if(texture == NULL)
			return RenderDevice::FAIL;

		*surface = new HWDepthStencilSurface();
		(*surface)->texture = texture;
		(*surface)->desc = *desc;

		return RenderDevice::SUCCESS;
	}

    RenderDevice::error_t RenderDevice::CreateDepthStencilSurface(HWTexture2D::Desc *texDesc, HWDepthStencilSurface::Desc *dsDesc, jade::HWDepthStencilSurface **surface)
    {
        HWTexture2D* texture = NULL;
        error_t err = CreateTexture2D(texDesc, NULL, &texture);
        
        if(err != SUCCESS)
            return err;
        
        err = CreateDepthStencilSurface(texture, dsDesc, surface);
        
        return err;
    }
    
	static GLint GetGLAddressMode(int addressMode)
	{
		switch(addressMode)
		{
		case TextureSamplerState::TEX_ADDRESS_WRAP:
			return GL_REPEAT;
			break;
		case TextureSamplerState::TEX_ADDRESS_CLAMP:
			return GL_CLAMP_TO_EDGE;
			break;
		case TextureSamplerState::TEX_ADDRESS_MIRROR:
			return GL_MIRRORED_REPEAT;
			break;
		}

		return GL_REPEAT;
	}

	static GLint GetGLCompareFunc(int comparisonFunc)
	{
		switch (comparisonFunc)
		{
			case TextureSamplerState::TEX_COMPARE_LESS:
				return GL_LEQUAL;
				break;
			case TextureSamplerState::TEX_COMPARE_LEQUAL:
				return GL_LEQUAL;
				break;
			case TextureSamplerState::TEX_COMPARE_EQUAL:
				return GL_EQUAL;
				break;
			case TextureSamplerState::TEX_COMPARE_GEQUAL:
				return GL_GEQUAL;
				break;
			case TextureSamplerState::TEX_COMPARE_GREATER:
				return GL_GREATER;
				break;
		}
		
		return GL_LEQUAL;
	}

#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT      0x84FF //core don't support anisotropic, define myself

	RenderDevice::error_t RenderDevice::CreateSamplerState(TextureSamplerState::Desc* desc, TextureSamplerState** state)
	{
		*state = new TextureSamplerState();

		(*state)->desc = *desc;
		switch(desc->filter)
		{
		case TextureSamplerState::TEX_FILTER_MIN_MAG_MIP_POINT:
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case TextureSamplerState::TEX_FILTER_MIN_MAG_MIP_LINEAR:
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case TextureSamplerState::TEX_FILTER_ANISOTROPIC:
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case TextureSamplerState::TEX_FILTER_SHADOW_MAP:
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_COMPARE_FUNC, GetGLCompareFunc(desc->comparisonFunc));

			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case TextureSamplerState::TEX_FILTER_PCF_SHADOW_MAP:
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_COMPARE_FUNC, GetGLCompareFunc(desc->comparisonFunc));

			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		}

		glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_WRAP_S, GetGLAddressMode(desc->uAddressMode) );
		glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_WRAP_T, GetGLAddressMode(desc->vAddressMode));
		glSamplerParameteri((*state)->impl.sampler, GL_TEXTURE_WRAP_R, GetGLAddressMode(desc->wAddressMode));

		if(desc->filter == TextureSamplerState::TEX_FILTER_ANISOTROPIC && desc->maxAnisotropy > 1)
		{
			glSamplerParameterf((*state)->impl.sampler, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,  (float)desc->maxAnisotropy);
		}
		return RenderDevice::SUCCESS;
	}

}

#endif