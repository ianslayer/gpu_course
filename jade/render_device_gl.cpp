#include "render_device.h"
#include "../window.h"

namespace jade
{
	GLenum GetGLSizedTexFormat(TEXTURE_FORMAT format)
	{
		switch(format)
		{
		case TEX_FORMAT_RGBA8:
			return GL_RGBA8;
		case TEX_FORMAT_RGBA16F:
			return GL_RGBA16F;
		case TEX_FORMAT_SRGB8_ALPHA8:
			return GL_SRGB8_ALPHA8;
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

	RenderDevice::error_t RenderDevice::CreateTexture2D(HWTexture2D::Desc* desc, SubresourceData* data, HWTexture2D** texture)
	{
		*texture = new HWTexture2D();
		glBindTexture(GL_TEXTURE_2D, (*texture)->GetImpl()->id);
		

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, desc->width, desc->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data->buf);
		
		glTexStorage2D(GL_TEXTURE_2D, desc->mipLevels, GetGLSizedTexFormat(desc->format), desc->width, desc->height);
		
		if(data && data->buf)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
			0,
			0, 0,
			desc->width, desc->height, 
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			data->buf
			);

			if(desc->generateMipmap)
				glGenerateMipmap(GL_TEXTURE_2D);
		}
		


		return RenderDevice::SUCCESS;
	}

}