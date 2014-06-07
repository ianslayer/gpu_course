#ifndef RENDERER_RAY_TRACING_H
#define RENDERER_RAY_TRACING_H
#include "geometry.h"

namespace jade
{
	class Scene;
	class Primitive;
	
	struct ImageSample
	{
		Vector2 xy;
	};
	
	struct Intersection
	{
		Vector3 p;
		Vector3 n;
		Vector2 uv;
		const Primitive* prim;
	};
	
	int Intersect(const Ray& ray, const Scene* scene, Intersection& isect);
	int Intersect(const Ray& ray, const Scene* scene);
	
	Vector3 Radiance(const Scene* scene, const Ray& ray);
	
}


#endif
