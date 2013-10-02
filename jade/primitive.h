#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "refcount.h"
#include "../matrix.h"
#include "../vector.h"

namespace jade
{
    class Mesh;
   // class Material;

    class Primitive : public RefCounted
    {
    public:
        Primitive();
        ~Primitive();

        Matrix4x4 ModelMatrix() const;

        RefCountedPtr<Mesh>  mesh;
       // Material*   material;

        Matrix3x3   orientation;
        Vector3     translation;
    };
}

#endif