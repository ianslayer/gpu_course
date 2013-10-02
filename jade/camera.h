#ifndef CAMERA_H
#define CAMERA_H

#include "../vector.h"
#include "../matrix.h"

namespace jade
{
    class Camera
    {
    public:

        Camera();

        Matrix4x4 ViewMatrix() const;
        Matrix4x4 PerspectiveMatrix() const;

        // view transform
        Vector3 position;

        Vector3 lookat;//view space -z
        Vector3 right; //view space +x
        Vector3 up;    //view space +y

        //projection
        float    n;
        float	 f;
        float	 l;
        float	 r;
        float	 t;
        float    b;
    };
}

#endif