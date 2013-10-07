#include "camera.h"

namespace jade
{
    Camera::Camera()
    {
        position = Vector3(0, 0, 0);
        lookat = Vector3(-1, 0, 0);
        right = Vector3(0, 1, 0);
        up = Vector3(0, 0, 1);

        n = 0.1f;
        f = 3000.f;
        l = -0.1f;
        r = 0.1f;
        t = 0.1f;
        b = -0.1f;
    }

    Matrix4x4 Camera::ViewMatrix() const
    {
        Matrix4x4 camAxis = Matrix4x4(right.x, up.x, -lookat.x, 0,
            right.y, up.y, -lookat.y, 0,
            right.z, up.z, -lookat.z, 0,
            0,        0,      0,      1);

        Matrix4x4 viewMatrix =  camAxis.Transpose() * Translate(-position) ;

        return viewMatrix;
    }

    Matrix4x4 Camera::PerspectiveMatrix() const
    {

        Matrix4x4 projMatrix = Matrix4x4(2.0f * n / (r - l), 0,					 (r + l) / (r - l), 0,
            0,					 2.0f * n / (t - b), (t + b) / (t - b), 0,
            0,					 0,				    -(f + n) / (f - n), -2.0f * f * n / (f - n),
            0,					 0,				    -1,               0);

        return projMatrix;
    }
}