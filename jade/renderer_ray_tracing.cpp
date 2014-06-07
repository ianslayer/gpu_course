#include "scene.h"
#include "geometry.h"
#include "mesh.h"
#include "primitive.h"
#include "renderer_ray_tracing.h"

namespace jade
{
	//inline int IntersectRayAABB(const Ray& ray, const AABB& bound, Range& range, float epsilon)
	
	int IntersectPrimitiveTriangle(const Ray& ray, const Range& range, const Primitive* prim, Intersection& isect)
	{
		float tmin = FLT_MAX;
		//inline int IntersectSegmentTriangle(const Ray& ray, const Range& range, const VertexType* vertices, const int* indices, float& u, float& v, float& w, float& t)
		for(size_t i = 0; i < prim->mesh->numIndices / 3; i++)
		{
			//if(IntersectSegmentTriangle(ray, range, ) )
			{
				
			}
		}
		
		return 0;
	}
	
	int Intersect(const Ray& ray, const Scene* scene, Intersection& isect)
	{
		Range range;
		float epsilon = 0.00001f;
		
		for(size_t i = 0 ; i < scene->primList.size(); i++)
		{
			const Primitive* prim = scene->primList[i].Get();
			AABB worldBound = prim->WorldBound();
			Range newRange;
			if(IntersectRayAABB(ray, worldBound, newRange, epsilon) == 1 )
			{
				
			}
		}
		
		return 0;
	}
}