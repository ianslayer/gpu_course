#include "scene.h"
#include "geometry.h"
#include "mesh.h"
#include "primitive.h"
#include "camera.h"
#include "brdf.h"
#include "light.h"
#include "renderer_ray_tracing.h"
#include "renderer.h"
#include "render_device.h"
#include "../window.h"
#include "../image.h"

namespace jade
{
	class TransformCache
	{
	public:
		TransformCache(): worldPosCache(0), worldNormalCache(0){}
		~TransformCache();

		Vertex*		worldPosCache;
		Vector3*    worldNormalCache;

		AABB		worldBound;
		const		Primitive* prim;
	};
	
	TransformCache::~TransformCache()
	{
		delete[] worldPosCache;
		delete[] worldNormalCache;
	}
	void BuildCache(const Primitive* prim, TransformCache& cache)
	{
		cache.prim = prim;
		delete [] cache.worldPosCache;
		delete [] cache.worldNormalCache;
		
		cache.worldPosCache = new Vertex[prim->mesh->numIndices];
		cache.worldNormalCache = new Vector3[prim->mesh->numIndices];
		
		Matrix4x4 transform = prim->ModelMatrix();
		
		for(int i = 0; i < prim->mesh->numIndices; i++)
		{
			cache.worldPosCache[i].position = DiscardW(transform * Vector4(prim->mesh->positionList[prim->mesh->indices[i]], 1.f));
		}
		
		for(int i = 0; i < prim->mesh->numIndices; i++)
		{
			cache.worldNormalCache[i] = DiscardW(transform * Vector4(prim->mesh->normalList[prim->mesh->indices[i]], 0.f));
		}
		cache.worldBound = prim->WorldBound();
	}
	
	struct Intersection
	{
		Vector3 p;
		Vector3 n;
		Vector4 tangent;
		Vector2 uv;
		float minT;
		const Primitive* prim;
	};

	struct IntersectInfo
	{
		float u, v; //barycentric coord of intersect
		float minT;
		int triIndex;
		const class TransformCache* cache;
		const Ray* ray;
	};

	void ResolveIntersection(const IntersectInfo& isectInfo, Intersection& isect)
	{
		isect.p = GetPoint(*isectInfo.ray, isectInfo.minT);

		Vector3 n0, n1, n2;
		Vector3 t0, t1, t2;

		float u = isectInfo.u;
		float v = isectInfo.v;
		float w = 1- u - v;

		n0 = isectInfo.cache->worldNormalCache[isectInfo.triIndex * 3];
		n1 = isectInfo.cache->worldNormalCache[isectInfo.triIndex * 3 + 1];
		n2 = isectInfo.cache->worldNormalCache[isectInfo.triIndex * 3 + 2];

		int i0, i1, i2; //tangent and other properties must be indexed by index buffer
		i0 = isectInfo.cache->prim->mesh->indices[isectInfo.triIndex * 3];
		i1 = isectInfo.cache->prim->mesh->indices[isectInfo.triIndex * 3 + 1];
		i2 = isectInfo.cache->prim->mesh->indices[isectInfo.triIndex * 3 + 2];

		float tangentParity = isectInfo.cache->prim->mesh->tangentList[i0][3];
		t0 = DiscardW(isectInfo.cache->prim->mesh->tangentList[i0]);
		t1 = DiscardW(isectInfo.cache->prim->mesh->tangentList[i1]);
		t2 = DiscardW(isectInfo.cache->prim->mesh->tangentList[i2]);

		isect.n = Normalize(n0 *u + n1 * v + n2 * w);
		isect.tangent = Vector4(Normalize(t0 *u + t1 * v + t2 * w),  tangentParity);
	}

	void IntersectPrimitiveTriangle(const Ray& ray, const Range& range, const TransformCache& cache, IntersectInfo& isectInfo)
	{
		float tmin = std::min(range.tmax, isectInfo.minT);

		for(int i = 0; i < cache.prim->mesh->numIndices / 3; i++)
		{
			float u, v, w;
			float t;

			int index[3];
			index[0] = i * 3 ;
			index[1] = i * 3 + 1;
			index[2] = i * 3 + 2;

			if(IntersectSegmentTriangle(ray, range, cache.worldPosCache, index, u, v, w, t) == 1)
			{
				if(t < tmin)
				{
					isectInfo.minT =tmin = t;
					isectInfo.triIndex = i;
					isectInfo.u = u;
					isectInfo.v = v;
					isectInfo.cache =&cache;
					isectInfo.ray = &ray;
				}
			}
		}
	}

	float SampleVisiblity(const Ray& ray, const Range& range, const TransformCache& cache)
	{
		for(int i = 0; i < cache.prim->mesh->numIndices / 3; i++)
		{
			float u, v, w;
			float t;
			
			int index[3];
			index[0] = i * 3;
			index[1] = i * 3 + 1;
			index[2] = i * 3 + 2;
			
			if(IntersectSegmentTriangle(ray, range, cache.worldPosCache, index, u, v, w, t) == 1 )
			{
				return 0.f;
			}
		}
		
		return 1.f;
	}
	
	class RayPacketIntersectJob
	{
		
	};
	
	class RayPacketIntersectResult
	{
		
	};
	
	class SceneAccelerator
	{
	public:
		virtual void Build(const Scene*) = 0;
		virtual void Intersect(const RayPacketIntersectJob& job, RayPacketIntersectResult& result) const{};
		
		virtual int Intersect(const Ray& ray, Intersection& isect) const = 0;
		virtual float Visibility(const Ray& ray, const Range& rayRange) const = 0;
	};
	
	class EmptySceneAcclerator : public SceneAccelerator
	{
	public:
		EmptySceneAcclerator();
		~EmptySceneAcclerator();
		
		virtual void Build(const Scene* scene);
		virtual int Intersect(const Ray& ray, Intersection& isect) const;
		virtual float Visibility(const Ray& ray, const Range& rayRange) const;
		
		TransformCache* cache;
		const Scene* scene;
	};
	
	EmptySceneAcclerator::EmptySceneAcclerator()
	: cache(0), scene(0)
	{
		
	}
	
	EmptySceneAcclerator::~EmptySceneAcclerator()
	{
		delete [] cache;
	}
	
	void EmptySceneAcclerator::Build(const jade::Scene *scene)
	{
		this->scene = scene;
		cache = new TransformCache[scene->primList.size()];
		for(size_t i = 0; i < scene->primList.size(); i++)
		{
			BuildCache(scene->primList[i], cache[i]);
		}
		
	}
	
	int EmptySceneAcclerator::Intersect(const jade::Ray &ray, jade::Intersection &isect) const
	{
		float epsilon = 0.00001f;
		IntersectInfo isectInfo;
		isectInfo.minT = FLT_MAX;
		isectInfo.triIndex = -1;
		for(size_t i = 0 ; i < scene->primList.size(); i++)
		{
			const AABB& worldBound = cache[i].worldBound;
			Range newRange;
			if(IntersectRayAABB(ray, worldBound, newRange, epsilon) == 1  )
			{
				IntersectPrimitiveTriangle(ray, newRange, cache[i], isectInfo);	
			}
		}
		
		if(isectInfo.triIndex != -1)
		{
			ResolveIntersection(isectInfo, isect);
			return 1;
		}

		return 0;
	}
	
	float EmptySceneAcclerator::Visibility(const jade::Ray &ray, const jade::Range &rayRange) const
	{
		float epsilon = 0.00001f;
		
		for(size_t i = 0 ; i < scene->primList.size(); i++)
		{
			const AABB& worldBound = cache[i].worldBound;
			Range newRange;
			if(IntersectRayAABB(ray, worldBound, newRange, epsilon) == 1 ) //fixme, should changed to segment to aabb test
			{
				if(SampleVisiblity(ray,  rayRange, cache[i]) == 0.f)
					return 0.f;
			}
		}
		
		return 1.f;
	}

	RGB32F Sample(const PointLight& light, const Vector3& p, Vector3& wi, float& pdf)
	{
		Vector3 l = light.pos - p;
		wi = Normalize(l);
		pdf = 1.f;
		return light.intensity / l.SquaredLength();
	}
	
	float SampleVisibility(const Vector3& p, const PointLight& light, float epsilon, const SceneAccelerator* accelerator)
	{
		Ray r;
		r.origin = p;
		Vector3 dir = light.pos - p;
		r.direction = Normalize(dir);
		
		Range range;
		range.tmin = epsilon;
		range.tmax = dir.Length();
		
		return accelerator->Visibility(r, range);
	}
	
	RGB32F Sample(const DirectionLight& light, const Vector3& p, Vector3& wi, float& pdf)
	{
		wi = light.dir;
		pdf = 1.f;
		return light.radiance;
	}
	
	int Intersect(const Ray& ray, const SceneAccelerator* accelerator, Intersection& isect)
	{
		return accelerator->Intersect(ray, isect);
	}
	
	float SampleVisibility(const Vector3& p, const DirectionLight& light, float epsilon, const SceneAccelerator* accelerator)
	{
		Ray r;
		r.origin = p;
		Vector3 dir = light.dir;
		r.direction = Normalize(dir);
		
		
		Range range;
		range.tmin = epsilon;
		range.tmax = 10000.f; //fixme, should be scene bound
		
		
		return accelerator->Visibility(r, range);
	}
	
	RGB32F Sample(const Light& light, const Vector3& p, Vector3& wi, float& pdf)
	{
		switch(light.type)
		{
			case Light::LT_POINT:
				return Sample((const PointLight&) light, p, wi, pdf);
			case Light::LT_DIRECTION:
				return Sample((const DirectionLight&) light, p, wi, pdf);
			case Light::LT_GEOMETRY_AREA:
				return RGB32F(0.f);
		}
		
		return RGB32F(0.f);
	}
	
	float SampleVisibility(const Vector3& p, const Light& light, float epsilon, const SceneAccelerator* accelerator)
	{
		switch(light.type)
		{
			case Light::LT_POINT:
				return SampleVisibility(p, (const PointLight&) light, epsilon, accelerator);
			case Light::LT_DIRECTION:
				return SampleVisibility(p, (const DirectionLight&) light, epsilon, accelerator);
			case Light::LT_GEOMETRY_AREA:
				return 1.f;
		}
		
		return 1.f;
	}


	class RendererRT : public Renderer
	{
	public:
		virtual ~RendererRT() {}
		virtual void Render(const Camera* camera, const Scene* scene) ;
		virtual void ScreenShot(const char* path, const Camera* camera, const Scene* scene);
		virtual void SetRendererOption(void* options);

		RTRenderOption options;
	};

	void RendererRT::Render(const Camera* camera, const Scene* scene)
	{

	}

	void SetColor(unsigned char* imgBuf, int width, int height, int x, int y, const Vector3& color)
	{
		Vector3 clampedColor;
		clampedColor[0] = std::max(color[0], 0.f);
		clampedColor[1] = std::max(color[1], 0.f);
		clampedColor[2] = std::max(color[2], 0.f);

		imgBuf[y * width * 4 + x  * 4] = std::min(clampedColor[2] * 255.f, 255.f);
		imgBuf[y * width * 4 + x  * 4 + 1] = std::min(clampedColor[1] * 255.f, 255.f);
		imgBuf[y * width * 4 + x  * 4 + 2] = std::min(clampedColor[0] * 255.f, 255.f);
		imgBuf[y * width * 4 + x  * 4 + 3] = 0;
	}

	Vector3 Shade(const Scene* scene, const SceneAccelerator* accelerator, const Ray& ray, const Intersection& isect)
	{
		Vector3 color = Vector3(0.f);
		for(int i = 0; i < scene->lightList.size(); i++)
		{
			Light* light = scene->lightList[i];
			
			Vector3 wi;
			float pdf;
			RGB32F lightRadiance = Sample(*light, isect.p, wi, pdf);
			
			float visibility = SampleVisibility(isect.p, *light, 0.01f, accelerator);
			
			float nDotL = std::max(dot(isect.n, wi), 0.f);
			Vector3 wo = Normalize(-ray.direction);
			
			color += lightRadiance * nDotL * visibility * BlinnBRDF(wo, wi, isect.n,  Vector3(0.04), 0.5) / pdf;
			
			color = isect.n * 0.5 + Vector3(0.5);
			//color = DivideW(isect.tangent) * 0.5 + Vector3(0.5);
		}

		return color;
	}

	void RendererRT::ScreenShot(const char* path, const Camera* camera, const Scene* scene)
	{
		unsigned char* imgBuf = new unsigned char[options.width * options.height * 4];
		
		SceneAccelerator* accelerator = new EmptySceneAcclerator();
		accelerator->Build(scene);
		

		Matrix4x4 rasterToCamMat = camera->RasterToCameraMatrix();
		Matrix4x4 invViewMatrix = camera->InvViewMatrix();

		for(int i = 0; i < options.height; i++)
		{
			for(int j = 0; j < options.width; j++)
			{
				Vector3 sampleCamPos = DivideW(rasterToCamMat * Vector4((float) j, (float) i, -1, 1) );
				Ray ray;
				ray.origin = Vector3(0.f);
				ray.direction = Normalize(sampleCamPos);

				ray = Transform(invViewMatrix, ray);

				Intersection isect;

				if(Intersect(ray, accelerator, isect))
				{
					Vector3 color = Shade(scene, accelerator, ray, isect);
					SetColor(imgBuf, options.width, options.height, j, i, color);
				}
				else
				{
					SetColor(imgBuf, options.width, options.height, j, i, Vector3(0.5, 0.5, 0.5));
				}

			}
		}

		SaveTGA(path, imgBuf,  options.width, options.height);
		
		delete accelerator;
		delete [] imgBuf;

	}

	void RendererRT::SetRendererOption(void* options)
	{
		this->options = *((RTRenderOption*)options);
	}

	void InitRendererRT(RenderDevice* device, Renderer** renderer)
	{
		RendererRT* rendererRT;
		*renderer = rendererRT = new RendererRT();
		RTRenderOption options;
		options.width = device->window->width;
		options.height = device->window->height;
		options.spp = 1;
		
		rendererRT->SetRendererOption(&options);
	}

	void ShutdownRendererRT(Renderer* renderer)
	{
		delete renderer;
	}
}