#include "scene.h"
#include "geometry.h"
#include "mesh.h"
#include "primitive.h"
#include "camera.h"
#include "material.h"
#include "brdf.h"
#include "light.h"
#include "tiled_image.h"
#include "renderer_ray_tracing.h"
#include "renderer.h"
#include "render_device.h"
#include "../window.h"
#include "../image.h"
#include "rng.h"

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
		isect.prim = isectInfo.cache->prim;
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
	public:
		int tileX;
		int tileY;
		Tile<Ray, 32>* rayPacket;
	};
	
	class RayPacketIntersectResult
	{
	public:
		int tileX;
		int tileY;
		Tile<IntersectInfo, 32>* isectResult;
	};
	
	class EmptySceneAcclerator : public SceneAccelerator
	{
	public:
		EmptySceneAcclerator();
		~EmptySceneAcclerator();
		
		virtual void Build(const Scene* scene);
		virtual int Intersect(const Ray& ray, Intersection& isect) const;
		virtual float Visibility(const Ray& ray, const Range& rayRange, float epsilon) const;
		
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
		delete [] cache;
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
	
	float EmptySceneAcclerator::Visibility(const jade::Ray &ray, const jade::Range &rayRange, float epsilon) const
	{
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

	class Voxel
	{
	public:
		Voxel();
		
		TransformCache* cacheList;
		int					numCaches;
	};
	
	class GridAccelerator: public SceneAccelerator
	{
	public:
		GridAccelerator(const Scene* scene);
		
		int		dim[3];
		AABB	bound;
	};

	RGB32F Sample(const PointLight& light, const Vector3& p, Vector3& wi, Range& lightRange, float& pdf)
	{
		Vector3 l = light.pos - p;
		wi = Normalize(l);
		pdf = 1.f;
		
		lightRange.tmin = 0.00001f;
		lightRange.tmax = l.Length();
		
		return light.intensity / l.SquaredLength();
	}
	
	
	RGB32F Sample(const DirectionLight& light, const Vector3& p, Vector3& wi, Range& lightRange, float& pdf)
	{
		wi = light.dir;
		pdf = 1.f;
		
		lightRange.tmin = 0.00001f;
		lightRange.tmax = 10000.f; //fixme, should be scene bound
		
		return light.radiance;
	}
	
	int Intersect(const Ray& ray, const SceneAccelerator* accelerator, Intersection& isect)
	{
		return accelerator->Intersect(ray, isect);
	}
	
	
	
	RGB32F Sample(const GeomAreaLight& light, const Vector3& p, float u1, float u2, float u3, Vector3& wi, Range& lightRange, float& pdf)
	{
		Vector3 normal;
		Vector3 pos = light.sampler.Sample(u1, u2, u3, normal);
		
		Vector3 l = pos - p;
		
		wi = Normalize(l);

		//get nearest point
		float thit;
		Ray ray(p, wi);
		Intersect(*light.prim, ray, 0.0001f, thit, normal);
		
		pdf = light.sampler.Pdf(p, wi);
		
		lightRange.tmin = 0.0001f;
		lightRange.tmax = thit - 0.0001f;
		
		RGB32F Ls =light.L(normal, -wi);
		
		return Ls;
	}
	
		RNG rng;
	
	RGB32F Sample(const Light& light, const Vector3& p, Vector3& wi, Range& range, float& pdf)
	{
		switch(light.type)
		{
			case Light::LT_POINT:
				return Sample((const PointLight&) light, p, wi, range, pdf);
			case Light::LT_DIRECTION:
				return Sample((const DirectionLight&) light, p, wi, range, pdf);
			case Light::LT_GEOMETRY_AREA:
				return  Sample((const GeomAreaLight&) light, p, rng.RandomFloat(), rng.RandomFloat(), rng.RandomFloat(), wi, range, pdf);;
		}
		
		return RGB32F(0.f);
	}
	
	float Pdf(const Light& light, const Vector3& p, const Vector3& wi)
	{
		switch(light.type)
		{
			case Light::LT_POINT:
			case Light::LT_DIRECTION:
				return 0.f;
			case Light::LT_GEOMETRY_AREA:
			{
				const GeomAreaLight& areaLight = (const GeomAreaLight&) light;
				return areaLight.sampler.Pdf(p, wi);
			}
		}
		
		return 0.f;
	}
	
	RGB32F Le(const Light& light, const Vector3& n, const Vector3& w)
	{
		switch(light.type)
		{
			case Light::LT_POINT:
				
			case Light::LT_DIRECTION:
				return Vector3(0.f);
			case Light::LT_GEOMETRY_AREA:
			{
				const GeomAreaLight& areaLight = (const GeomAreaLight&) light;
				return areaLight.L(n, w);
			}
		}
		
		return Vector3(0.f);
	}
	
	
	float SampleVisibility(const Ray& ray, const Range& range, float epsilon,  const SceneAccelerator* accelerator)
	{
		return accelerator->Visibility(ray, range, epsilon);
	}
	
	class RendererRT : public Renderer
	{
	public:
		RendererRT() : accelerator(0) {}
		virtual ~RendererRT() 
		{
			delete accelerator;
		}

		virtual void Render(const Camera* camera, const Scene* scene) ;
		virtual void ScreenShot(const char* path, const Camera* camera, const Scene* scene);
		virtual void SetRendererOption(void* options);
		virtual class SceneAccelerator* GetAccelerator() { return accelerator; };

		RTRenderOption options;

		SceneAccelerator* accelerator;
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
	
	// Monte Carlo Inline Functions
	inline float BalanceHeuristic(int nf, float fPdf, int ng, float gPdf) {
		return (nf * fPdf) / (nf * fPdf + ng * gPdf);
	}
	
	
	inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf) {
		float f = nf * fPdf, g = ng * gPdf;
		return (f*f) / (f*f + g*g);
	}
	
	RGB32F DirectLightUniformSample(const Scene* scene, const SceneAccelerator* accelerator, const Ray& ray, const Intersection& isect)
	{
		RGB32F L(0.f);
		

		Vector3 wo = -ray.direction;
	
		if(isect.prim->areaLight)
		{
			L += isect.prim->areaLight->L(isect.n, wo);
		}
		
		
		int nSamples = 1024;
		
		Vector3 Ld(0.f);
		for(int i = 0; i < nSamples; i++)
		{
			
			Vector3 tangentDir = UniformSampleHemisphere(rng.RandomFloat(), rng.RandomFloat());
			Vector3 wi = TransformToWorldSpace(isect.n, isect.tangent, tangentDir);
			
			Intersection lightIsect;
			Ray sampleRay(isect.p + 0.0001f * wi, wi);
			if(Intersect(sampleRay, accelerator, lightIsect) )
			{
				if(lightIsect.prim->areaLight)
				{
					Vector3 Li = lightIsect.prim->areaLight->radiance;
					Vector3 f =  BlinnBRDF(wo, wi, isect.n, Vector3(1.00,0.71,0.29), 1024.f);
					
					Ld+=f * Li * abs(dot(wi, isect.n)) / UniformHemispherePdf();
				}
			}
		}
		
		
		L += Ld /nSamples;
		return L;
	}
	
	RGB32F DirectLighting(const Scene* scene, const SceneAccelerator* accelerator, const Ray& ray, const Intersection& isect)
	{
		RGB32F L(0.f);
		
		Vector3 wo = -ray.direction;
		
		if(isect.prim->areaLight)
		{
			L += isect.prim->areaLight->L(isect.n, wo);
		}
	
		int nSamples = 16;
		
		for(size_t i = 0; i < scene->lightList.size(); i++)
		{
			Vector3 Ld = Vector3(0.f);
			
			for(int j = 0; j < nSamples; j++)
			{
				
				//sample light
				Vector3 wi;

				float lightPdf, brdfPdf;
				Range lightRange;
				
				Vector3 Li = Sample(*scene->lightList[i], isect.p, wi, lightRange, lightPdf);
				Vector3 tangentWo = TransformToTangentSpace(isect.n, isect.tangent, wo);
				Vector3 tangentWi = TransformToTangentSpace(isect.n, isect.tangent, wi);
				
			
				if(lightPdf > 0.f)
				{

					Ray shadowRay(isect.p, wi);
					float vis = SampleVisibility(shadowRay, lightRange, 0.0001f, accelerator);
					Li *= vis;
					
					Vector3 f = BlinnBRDF(tangentWo, tangentWi, Vector3(1.00,0.71,0.29), 1024.f);
					
					if(IsDeltaLight(*scene->lightList[i]))
						Ld+=f * Li * abs(dot(wi, isect.n)) / lightPdf;

					else
					{
						brdfPdf = BlinnPdf(tangentWo, tangentWi, 1024.f);
						float weight = PowerHeuristic(1, lightPdf, 1, brdfPdf);

						Ld += f * Li * (std::max(dot(wi, isect.n), 0.f)  * weight / lightPdf);
				
					}

				}
				
				if(!IsDeltaLight(*scene->lightList[i]))
				{
					Vector3 tangentWi;
					SampleBlinnNDF(tangentWo, tangentWi, 1024.f, rng.RandomFloat(), rng.RandomFloat(), brdfPdf);

					Vector3 f = BlinnBRDF(tangentWo, tangentWi, Vector3(1.00,0.71,0.29), 1024.f);
					
					wi = TransformToWorldSpace(isect.n, isect.tangent, tangentWi);
					float weight = 1.f;
					lightPdf = Pdf(*scene->lightList[i], isect.p, wi);
					if(lightPdf != 0.f && brdfPdf > 0.f)
					{
					
						weight = PowerHeuristic(1, brdfPdf, 1, lightPdf);
						
						Ray ray(isect.p + 0.00001 * wi, wi);
						Intersection lightIsect;
						
						if(Intersect(ray, accelerator, lightIsect) )
						{
							if(lightIsect.prim->areaLight == scene->lightList[i])
							{
								Li = Le(*scene->lightList[i], lightIsect.n, -wi);
							}
						}
						else
							Li = Vector3(0.f);
						
							Ld += f * Li * std::max(dot(wi, isect.n), 0.f) * weight/brdfPdf;
			
					}
				}
				
			}

			L += Ld / nSamples;

			
		}
		
		return L;
	}
	
	Vector3 Shade(const Scene* scene, const SceneAccelerator* accelerator, const Ray& ray, const Intersection& isect)
	{
		Vector3 color = Vector3(0.f);
		for(int i = 0; i < scene->lightList.size(); i++)
		{
			Light* light = scene->lightList[i];
			
			Vector3 wi;
			float pdf;
			Range range;
			RGB32F lightRadiance = Sample(*light, isect.p, wi, range, pdf);
			
			Ray shadowRay;
			shadowRay.origin = isect.p;
			shadowRay.direction = wi;
			
			float visibility = SampleVisibility(shadowRay, range, 0.01f, accelerator);
			
			float nDotL = std::max(dot(isect.n, wi), 0.f);
			Vector3 wo = Normalize(-ray.direction);
			
			color += lightRadiance * nDotL * visibility * BlinnBRDF(wo, wi, isect.n,  Vector3(0.04), 0.5) / pdf;
			
			color = isect.n * 0.5 + Vector3(0.5);
			//color = DivideW(isect.tangent) * 0.5 + Vector3(0.5);
		}

		return color;
	}
	
	class SceneLightSampler
	{
	public:
		SceneLightSampler() : powerDistribution(0)
		{
			
		}
		
		SceneLightSampler(const Scene* scene)
		{
			std::vector<float> lightPowerList;
			for(size_t i = 0; i < scene->lightList.size(); i++)
			{
				lightPowerList.push_back(scene->lightList[i]->power.y);
			}
			powerDistribution = new Distribution1D(&lightPowerList[0], lightPowerList.size() );
		}
		
		~SceneLightSampler()
		{
			delete powerDistribution;
		}
		
		Distribution1D* powerDistribution;
		std::vector<int> lightSampleCount;
	};
	
	void RendererRT::ScreenShot(const char* path, const Camera* camera, const Scene* scene)
	{
		unsigned char* imgBuf = new unsigned char[options.width * options.height * 4];
		
		accelerator->Build(scene);

		Matrix4x4 rasterToCamMat = camera->RasterToCameraMatrix();
		Matrix4x4 invViewMatrix = camera->InvViewMatrix();

		Material defaultMaterial;
		//defaultMaterial.
		
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
					Vector3 color = DirectLighting(scene, accelerator, ray, isect);// DirectLighting(scene, accelerator, ray, isect);
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
		rendererRT->accelerator = new EmptySceneAcclerator();
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