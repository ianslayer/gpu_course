#ifndef SCENE_H
#define SCENE_H
#include <vector>
#include "refcount.h"

namespace jade
{
    class Primitive;
	class Light;

    class Scene
    {
    public:

        void AddPrimitives(const std::vector<Primitive*>& primList);
		void AddLight(Light* light);

        std::vector<RefCountedPtr<Primitive> > primList;
		std::vector<RefCountedPtr<Light> > lightList;
    };
	
}

#endif