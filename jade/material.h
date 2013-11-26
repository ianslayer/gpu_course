#ifndef MATERIAL_H
#define MATERIAL_H
#include "../vector.h"
#include "texture.h"

namespace jade
{
	class Material : public RefCounted
	{
	public:

		Material() : ambient(0.f), diffuse(1.f), specular(1.f), roughness(0.f) {}

		Vector3 ambient;
		Vector3 diffuse;
		Vector3 specular;
		float	roughness;

		RefCountedPtr<Texture> lightMap;
		RefCountedPtr<Texture> diffuseMap;
		RefCountedPtr<Texture> specularMap;
		RefCountedPtr<Texture> roughnessMap;
		RefCountedPtr<Texture> normalMap;
		RefCountedPtr<Texture> dissolveMask;
	};

}


#endif