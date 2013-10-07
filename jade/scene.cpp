#include "scene.h"
#include "primitive.h"

namespace jade
{
	void Scene::AddPrimitives(const std::vector<RefCountedPtr<Primitive> >& _primList)
	{
		for(size_t i = 0; i < _primList.size(); i++)
		{
			primList.push_back(_primList[i]);
		}
	}
}