#ifndef _CAMERA_H
#define _CAMERA_H

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
	Matrix4x4 Perspective() const;

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

inline MyPerspectiveCamera::MyPerspectiveCamera()
{
	position = Vector3(0, 0, 0);
	lookat = Vector3(-1, 0, 0);
	right = Vector3(0, 1, 0);
	up = Vector3(0, 0, 1);

	n = 1.f;
	f = 1000.f;
	l = -1;
	r = 1;
	t = 1;
	b = -1;
}

inline MyPerspectiveCamera::MyPerspectiveCamera(const Vector3& _position, const Vector3& lookat, const Vector3& right, const Vector3& up)
{

}

inline Matrix4x4 MyPerspectiveCamera::ViewMatrix() const
{
	Matrix4x4 camAxis = Matrix4x4(right.x, up.x, -lookat.x, 0,
		right.y, up.y, -lookat.y, 0,
		right.z, up.z, -lookat.z, 0,
		0,        0,      0,      1);

	Matrix4x4 viewMatrix =  camAxis.Transpose() * Translate(-position) ;

	return viewMatrix;
}

inline Matrix4x4 MyPerspectiveCamera::SimplePerspective() const
{
	Matrix4x4 projMatrix = Matrix4x4(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, -1.f / n, 0);

	return projMatrix;
}


inline Matrix4x4 MyPerspectiveCamera::Perspective() const
{

	Matrix4x4 projMatrix = Matrix4x4(2.0f * n / (r - l), 0,					 (r + l) / (r - l), 0,
		0,					 2.0f * n / (t - b), (t + b) / (t - b), 0,
		0,					 0,				    -(f + n) / (f - n), -2.0f * f * n / (f - n),
		0,					 0,				    -1,               0);

	return projMatrix;
}

#endif