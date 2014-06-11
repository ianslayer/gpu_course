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
	}

	void Scene::AddPrimitive(Primitive* prim)
	{
		primList.push_back(prim);
	}

	void Scene::AddLight(Light* light)
	{
		lightList.push_back(light);
	}
}