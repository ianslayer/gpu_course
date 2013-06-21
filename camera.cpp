#include "camera.h"

MyPerspectiveCamera::MyPerspectiveCamera()
{
    position = Vector3(0, 0, 0);
    lookat = Vector3(-1, 0, 0);
    right = Vector3(0, 1, 0);
    up = Vector3(0, 0, 1);

    zNear = 0.5f;
}

MyPerspectiveCamera::MyPerspectiveCamera(const Vector3& _position, const Vector3& lookat, const Vector3& right, const Vector3& up)
{

}

Matrix4x4 MyPerspectiveCamera::ViewMatrix() const
{
    Matrix4x4 camAxis = Matrix4x4(right.x, up.x, -lookat.x, 0,
                                  right.y, up.y, -lookat.y, 0,
                                  right.z, up.z, -lookat.z, 0,
                                  0,        0,      0,      1);

    Matrix4x4 viewMatrix =  camAxis.Transpose() * Translate(-position) ;

    return viewMatrix;
}

Matrix4x4 MyPerspectiveCamera::SimplePerspective() const
{
    Matrix4x4 projMatrix = Matrix4x4(1, 0, 0, 0,
                                     0, 1, 0, 0,
                                     0, 0, 1, 0,
                                     0, 0, -1.f / zNear, 0);

    return projMatrix;
}