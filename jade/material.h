#ifndef MATERIAL_H
#define MATERIAL_H
#include "../vector.h"
#include "texture.h"

namespace jade
{
	class Material : public RefCounted
	{
	public:

		Material() : ambient(0.f), diffuse(0.5f), specular(0.5f), roughness(0.5f) {}

		Vector3 ambient;
		Vector3 diffuse;
		Vector3 specular;
		float	roughness;

		RefCountedPtr<Texture> lightMap;
		RefCountedPtr<Texture> diffuseMap;
		RefCountedPtr<Texture> specularMap;
		RefCountedPtr<Texture> normalMap;
		RefCountedPtr<Texture> dissolveMask;
	};

}


#endif