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

        void AddPrimitives(const std::vector<RefCountedPtr<Primitive> >& primList);

        std::vector<RefCountedPtr<Primitive> > primList;
    };

}

#endif