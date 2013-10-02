#ifndef SCENE_H
#define SCENE_H
#include <vector>
#include "refcount.h"

namespace jade
{
    class Primitive;

    class Scene
    {
    public:

        void AddPrimitive(Primitive* prim);

        std::vector<RefCountedPtr<Primitive> > primList;
    };

}

#endif