#ifndef SCENE_H
#define SCENE_H
#include <vector>
#include "refcount.h"
#include "geometry.h"

namespace jade
{
    class Primitive;
	class Light;

    class Scene
    {
    public:

        void AddPrimitives(const std::vector<Primitive*>& primList);
		void AddPrimitive(Primitive* prim);
		void AddLight(Light* light);

		void ComputeWorldBound();

        std::vector<RefCountedPtr<Primitive> > primList;
		std::vector<RefCountedPtr<Light> > lightList;

		AABB worldBound;
    };
	
}

#endif