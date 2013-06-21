#ifndef _CAMERA_H

#include "vector.h"
#include "matrix.h"

class MyOrthoCamera
{

};

class MyPerspectiveCamera
{
public:
    MyPerspectiveCamera();
    MyPerspectiveCamera(const Vector3& _position, const Vector3& lookat, const Vector3& right, const Vector3& up);

    Matrix4x4 ViewMatrix() const;
    Matrix4x4 SimplePerspective() const;

    // view transform
    Vector3 position;

    Vector3 lookat;//view space -z
    Vector3 right; //view space +x
    Vector3 up;    //view space +y

    //projection
    float    zNear;

};

#endif