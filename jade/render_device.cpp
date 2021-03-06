#include "render_device.h"

namespace jade
{

int TotalMipLevels(int width, int height)
{
	int totalLevelsX = 1;
	int totalLevelsY = 1;

	while(width > 1)
	{
		width /= 2;
		totalLevelsX++;
	}
	while(height > 1)
	{
		height /= 2;
		totalLevelsY++;
	}
	return std::max(totalLevelsX, totalLevelsY);		
}

HWVertexBuffer::HWVertexBuffer() : size(0)
{

}

HWVertexBuffer::~HWVertexBuffer()
{

}

size_t HWVertexBuffer::Size() const
{
    return size;
}

const VertexBufferImpl* HWVertexBuffer::GetImpl() const
{
	return &impl;
}

HWIndexBuffer::HWIndexBuffer() : size(0)
{

}

HWIndexBuffer::~HWIndexBuffer()
{

}

size_t HWIndexBuffer::Size() const
{
    return size;
}

size_t HWIndexBuffer::IndexCount() const
{
    return size / sizeof(int);
}

const IndexBufferImpl* HWIndexBuffer::GetImpl() const
{
	return &impl;
}

const HWTexture2D::Desc* HWTexture2D::GetDesc() const
{
    return &desc;
}
    
const Texture2DImpl* HWTexture2D::GetImpl() const
{
	return &impl;
}

const HWRenderTexture2D::Desc* HWRenderTexture2D::GetDesc() const
{
	return &desc;
}

HWTexture2D* HWRenderTexture2D::GetTexture() const
{
	return texture;
}

const HWDepthStencilSurface::Desc* HWDepthStencilSurface::GetDesc() const
{
	return &desc;
}

HWTexture2D* HWDepthStencilSurface::GetTexture() const
{
	return texture;
}

const DepthStencilSurfaceImpl* HWDepthStencilSurface::GetImpl() const
{
	return &impl;
}

TextureSamplerState::TextureSamplerState()
{
        
}
    
TextureSamplerState::~TextureSamplerState()
{
        
}
  
    
TextureSamplerStateImpl* TextureSamplerState::GetImpl()
{
    return &impl;
}
    
    
}