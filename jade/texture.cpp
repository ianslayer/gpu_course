#include "texture.h"
#include "../image.h"
#include "../file_utility.h"

namespace jade
{
	Texture::Texture(TextureManager* _manager)
		: texBuf(0), manager(_manager)
	{

	}

	Texture::Texture()
		: texBuf(0), manager(0)
	{

	}

	Texture::~Texture()
	{
		if(manager)
			manager->RemoveTexture(path);
		delete [] texBuf;
	}

	bool Texture::Load(RenderDevice* device, const std::string& path, bool srgbTexture)
	{
		int width, height;
		unsigned char* imgbuf;

		bool loadStat = LoadTGA(path.c_str(), &imgbuf, &width, &height);

		HWTexture2D* _hwTexture = NULL;

		HWTexture2D::Desc desc;
		desc.arraySize = 0;
		if(srgbTexture)
			desc.format = TEX_FORMAT_SRGB8_ALPHA8;
		else
			desc.format = TEX_FORMAT_RGBA8;
		desc.width = width;
		desc.height = height;
		desc.mipLevels = (unsigned int) TotalMipLevels(width, height);
		desc.generateMipmap = true;

		SubresourceData data;
		data.buf = imgbuf;

		RenderDevice::error_t err = device->CreateTexture2D(&desc, &data, &_hwTexture);
		this->hwTexture = _hwTexture;
		
		return loadStat;
	}

	TextureManager::TextureManager(RenderDevice* _device)
	{
		device = _device;
		Texture* whiteTexture = new Texture();

		unsigned char white[] = {255, 255, 255, 255};

		HWTexture2D::Desc desc;
		desc.arraySize = 0;
		desc.format = TEX_FORMAT_RGBA8;
		desc.width = 1;
		desc.height = 1;
		desc.mipLevels = 1;
		desc.generateMipmap = true;

		SubresourceData data;
		data.buf = white;
		HWTexture2D* hwTexture = NULL;
		RenderDevice::error_t err = device->CreateTexture2D(&desc, &data, &hwTexture);

		whiteTexture->hwTexture = hwTexture;

		texTable["sys:white"] = whiteTexture;
	}

	Texture* TextureManager::Load(const std::string& path, bool srgbTexture)
	{
        std::string nPath = NormalizePath(path);
		int width, height;
		unsigned char* imgbuf;

		if(texTable.find(nPath) != texTable.end())
		{
			return texTable[nPath];
		}

		bool success = LoadTGA(nPath.c_str(), &imgbuf, &width, &height);

		if(!success)
		{
			printf("can't open image file: %s\n", nPath.c_str());
			return NULL;
		}

		Texture* texture = new Texture(this);
		texTable[nPath] = texture;
		texture->path = nPath;
		texture->texBuf = imgbuf;

		HWTexture2D* hwTexture = NULL;

		HWTexture2D::Desc desc;
		desc.arraySize = 0;
		if(srgbTexture)
			desc.format = TEX_FORMAT_SRGB8_ALPHA8;
		else
			desc.format = TEX_FORMAT_RGBA8;
		desc.width = width;
		desc.height = height;
		desc.mipLevels = (unsigned int) TotalMipLevels(width, height);
		desc.generateMipmap = true;

		SubresourceData data;
		data.buf = imgbuf;

		RenderDevice::error_t err = device->CreateTexture2D(&desc, &data, &hwTexture);
		texture->hwTexture = hwTexture;

		return texture;
	}

	void TextureManager::RemoveTexture(const std::string& path)
	{
		if(texTable.find(path) != texTable.end())
		{
			 texTable.erase(path);
		}
	}

}