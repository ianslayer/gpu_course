#ifndef LIGHT_H
#define LIGHT_H
#include "refcount.h"
#include "../vector.h"
#include "../matrix.h"
#include "../geometry.h"

namespace jade
{
	class Light : public RefCounted
	{
	public:
		enum TYPE
		{
			LT_POINT,
			LT_DIRECTION
		};

		Light(TYPE _type) : type(_type)
		{

		}

		const TYPE type;
	};

	class PointLight : public Light
	{
	public:

		PointLight() : pos(0.f, 0.f, 10.f), intensity(1.f), radius(1.f), Light(LT_POINT) { };
		PointLight(const Vector3& _pos, const Vector3 _intensity, float _radius) : pos(_pos), intensity(_intensity), radius(_radius), Light(LT_POINT) {};

		enum SHADOW_VIEW
		{
			POSITIVE_X,
			NEGATIVE_X,
			POSITIVE_Y,
			NEGATIVE_Y,
			POSITIVE_Z,
			NEGATIVE_Z
		};
		Matrix4x4 ShadowViewMatrix(int dir) const;
		Matrix4x4 ShadowProjMatrix() const;

		Vector3 pos;
		Vector3 intensity;
		float   radius;

	};

	class DirectionLight : public Light
	{
	public:

		DirectionLight() : dir(0, 0, 1), intensity(1.f), Light(LT_DIRECTION) {}
		DirectionLight(const Vector3& _dir, const Vector3 _intensity) : dir(_dir), intensity(_intensity), Light(LT_DIRECTION) {}
		Matrix4x4 ShadowViewMatrix() const;
		Matrix4x4 InvShadowViewMatrix() const;
		Matrix4x4 ShadowProjMatrix(const AABB& worldBound) const;

		Vector3 dir;
		Vector3 intensity;
	};
}

#endif