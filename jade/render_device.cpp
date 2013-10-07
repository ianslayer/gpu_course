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

VertexBufferImpl* HWVertexBuffer::GetImpl()
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

IndexBufferImpl* HWIndexBuffer::GetImpl()
{
	return &impl;
}

const Texture2DImpl* HWTexture2D::GetImpl() const
{
	return &impl;
}

Texture2DImpl* HWTexture2D::GetImpl()
{
	return &impl;
}


}