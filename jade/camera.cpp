#include "camera.h"

namespace jade
{
    Camera::Camera()
    {
        position = Vector3(0, 0, 0);
        lookat = Vector3(-1, 0, 0);
        right = Vector3(0, 1, 0);
        up = Vector3(0, 0, 1);

        xFov = 3.14f / 2.f;
        width = 1024.f;
        height = 1024.f;
        
        n = 0.1f;
        f = 3000.f;

        DeriveProjectionParamter();

    }

    void Camera::SetAspectRatio(float _width, float _height)
    {
        width = _width;
        height = _height;
        
        DeriveProjectionParamter();
    }
    
    void Camera::SetXFov(float angleRad)
    {
        xFov = angleRad;
        
        DeriveProjectionParamter();
    }
    
    void Camera::DeriveProjectionParamter()
    {
        r = n * tan(xFov/2.f);
        l = -r;
        
        t = r * (height / width);
        b = -t;
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

	Matrix4x4 Camera::InvViewMatrix() const
	{
		Matrix4x4 viewMat = ViewMatrix();
		return InverseAffine(viewMat);
	}
	
    Matrix4x4 Camera::PerspectiveMatrix() const
    {

        Matrix4x4 projMatrix = Matrix4x4(2.0f * n / (r - l), 0,					 (r + l) / (r - l), 0,
            0,					 2.0f * n / (t - b), (t + b) / (t - b), 0,
            0,					 0,				    -(f + n) / (f - n), -2.0f * f * n / (f - n),
            0,					 0,				    -1,               0);

        return projMatrix;
    }
	
	
	Matrix4x4 Camera::InvPerspectiveMatrix() const
	{
		Matrix4x4 invertProjMatrix = Matrix4x4((r-l)/(2*n), 0, 0, (r+l)/(2*n),
											   0, (t-b)/(2*n), 0, (t+b)/(2*n),
											   0, 0, 0, -1,
											   0, 0, -(f-n)/(2*f*n), (f+n)/(2*f*n));
		return invertProjMatrix;
	}

	Matrix4x4 Camera::ScreenToRasterMatrix() const
	{
		float window[4] = {-1, 1, -1, 1};
		return	Scale(Vector3(width, height, 1.f) )
				* Scale(Vector3(1.f / (window[1] - window[0]), 1.f / (window[3] - window[2]), 1.f ) ) 
				* Translate(Vector3(-window[0], -window[2], 0.f));
	}

	Matrix4x4 Camera::RasterToScreenMatrix() const
	{
		return Inverse(ScreenToRasterMatrix());
	}

	Matrix4x4 Camera::RasterToCameraMatrix() const
	{
		return InvPerspectiveMatrix() * RasterToScreenMatrix();
	}

}