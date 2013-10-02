#include "render_device.h"

namespace jade
{

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

}