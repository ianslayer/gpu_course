#ifndef MATERIAL_H
#define MATERIAL_H
#include "texture.h"

namespace jade
{
	class Material : public RefCounted
	{
	public:

		RefCountedPtr<Texture> diffuseTex;
	};

}


#endif