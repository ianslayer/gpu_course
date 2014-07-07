#include "scene.h"
#include "primitive.h"
#include "light.h"

namespace jade
{
	void Scene::AddPrimitives(const std::vector<Primitive*>& _primList)
	{
		for(size_t i = 0; i < _primList.size(); i++)
		{
			primList.push_back(_primList[i]);
		}

		ComputeWorldBound();
	}

	void Scene::AddPrimitive(Primitive* prim)
	{
		primList.push_back(prim);
		ComputeWorldBound();
	}

	void Scene::AddLight(Light* light)
	{
		lightList.push_back(light);
		
		if(light->type == Light::LT_GEOMETRY_AREA)
		{
			GeomAreaLight* geomLight = (GeomAreaLight*) light;
			primList.push_back(geomLight->prim.Get());
		}
	}

	void Scene::ComputeWorldBound()
	{
		for(size_t i = 0; i < primList.size(); i++)
		{
			worldBound = Merge(worldBound, primList[i]->WorldBound());
		}
	}
}