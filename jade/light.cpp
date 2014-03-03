#include "light.h"
#include "camera.h"

namespace jade
{
	Matrix4x4 DirectionLight::ShadowViewMatrix() const
	{
		Vector3 lookAt = -dir;
		Vector3 _right = Vector3(1, 0, 0);
		Vector3 _up = cross(_right, lookAt);

		if( _up.Length() < 0.1f )
		{
			_right = Vector3(0, 1, 0);
			_up = cross(_right, lookAt);

			if( _up.Length() < 0.1f )
			{
				_right = Vector3(0, 0, 1);
				_up = cross(_right, lookAt);
			}
		}

		_up.Normalize();
		_right = cross(lookAt, _up);

		Camera cam;
		cam.position = Vector3(0.f);
		cam.lookat = lookAt;
		cam.right = _right;
		cam.up = _up;

		return cam.ViewMatrix();

	}

	Matrix4x4 DirectionLight::InvShadowViewMatrix() const
	{
        Matrix4x4 shadowMatrix = ShadowViewMatrix();
		return InverseAffine(shadowMatrix);
	}

	Matrix4x4 DirectionLight::ShadowProjMatrix(const AABB& worldBound) const
	{

        Matrix4x4 shadowMatrix = ShadowViewMatrix();
		AABB viewBound = Transform(shadowMatrix, worldBound);

		Vector3 radius = viewBound.radius;
		Vector3 center = viewBound.center;

		float xmax = center.x + radius.x;
		float xmin = center.x - radius.x;
		float ymax = center.y + radius.y;
		float ymin = center.y - radius.y;
		float zmax = -(center.z - radius.z);
		float zmin = -(center.z + radius.z);

		float width = xmax - xmin;
		float height= ymax - ymin;
		
		return Matrix4x4(2.f / width, 0, 0, -(xmax + xmin) / width,
						 0, 2.f / height, 0, -(ymax + ymin) / height,
						 0, 0, -2.f / (zmax - zmin), -(zmax + zmin) / (zmax - zmin),
						 0, 0, 0, 1);
	}

}