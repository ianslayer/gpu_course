#ifndef RENDERER_RAY_TRACING_H
#define RENDERER_RAY_TRACING_H
#include "geometry.h"
#include "renderer.h"

namespace jade
{
	struct RTRenderOption
	{
		int width;
		int height;
		int spp;
	};

	struct Intersection
	{
		Vector3 p;
		Vector3 n;
		Vector4 tangent;
		Vector2 uv;
		const Primitive* prim;
	};

	class SceneAccelerator
	{
	public:
		virtual void Build(const Scene*) = 0;
		//virtual void Intersect(const RayPacketIntersectJob& job, RayPacketIntersectResult& result) const{};

		virtual int Intersect(const Ray& ray, Intersection& isect) const = 0;
		virtual float Visibility(const Ray& ray, const Range& rayRange, float epsilon) const = 0;

		//		SceneAccelerator* impl;
	};
	RGB32F Sample(const Light& light, const Vector3& p, Vector3& wi, Range& range, float& pdf);

	int Intersect(const Ray& ray, const SceneAccelerator* accelerator, Intersection& isect);
	float SampleVisibility(const Ray& ray, const Range& range, float epsilon,  const SceneAccelerator* accelerator);

	void InitRendererRT(class RenderDevice* device, Renderer** renderer);
	void ShutdownRendererRT(Renderer* renderer);
}


#endif
