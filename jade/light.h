#ifndef LIGHT_H
#define LIGHT_H
#include "refcount.h"

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

		PointLight() : pos(0.f, 0.f, 10.f), intensity(1.f), Light(LT_POINT) { };
		PointLight(const Vector3& _pos, const Vector3 _intensity) : pos(_pos), intensity(_intensity), Light(LT_POINT) {};

		Vector3 pos;
		Vector3 intensity;


	};

	class DirectionLight : public Light
	{
	public:

		DirectionLight() : dir(0, 0, 1), intensity(1.f), Light(LT_DIRECTION) {}
		DirectionLight(const Vector3& _dir, const Vector3 _intensity) : dir(_dir), intensity(_intensity), Light(LT_DIRECTION) {}

		Vector3 dir;
		Vector3 intensity;
	};
}

#endif