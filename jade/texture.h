#ifndef TEXTURE_H
#define TEXTURE_H
#include <string>
#include <map>
#include "render_device.h"

namespace jade
{
	class Texture : public RefCounted
	{
	public:
		Texture(class TextureManager* manager);
		Texture();
		~Texture();

		bool Load(RenderDevice* device, const std::string& path, bool srgbTexture = false);

		std::string path;

		unsigned char* texBuf;
		RefCountedPtr<HWTexture2D> hwTexture;
		class TextureManager* manager;
	};

	class TextureManager 
	{
	public:
		TextureManager(RenderDevice* device);

		Texture* Load(const std::string& path, bool srgbTexture = false);

		void RemoveTexture(const std::string& name);

		RenderDevice* device;

		std::map<std::string, Texture*> texTable;
	};
}


#endif