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
	struct Intersection
	{
		Vector3 p;
		Vector3 n;
		Vector2 uv;
		const Primitive* prim;
	};

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
	void BuildCache(const Camera* cam, const Primitive* prim, TransformCache& cache)
	{
		cache.prim = prim;
		delete [] cache.worldPosCache;
		delete [] cache.worldNormalCache;

		cache.worldPosCache = new Vertex[prim->mesh->numIndices];
		cache.worldNormalCache = new Vector3[prim->mesh->numIndices];

		Matrix4x4 viewMat = cam->ViewMatrix();
		Matrix4x4 transform = prim->ModelMatrix();
		
		for(int i = 0; i < prim->mesh->numIndices; i++)
		{
			cache.worldPosCache[i].position = DiscardW(transform * Vector4(prim->mesh->positionList[prim->mesh->indices[i]], 1.f));
		}
		
		for(int i = 0; i < prim->mesh->numIndices; i++)
		{
			//cache.worldPosCache[i].position = DiscardW(transform * Vector4(prim->mesh->positionList[i], 1.f));
			cache.worldNormalCache[i] = DiscardW(transform * Vector4(prim->mesh->normalList[prim->mesh->indices[i]], 0.f));
		}
		cache.worldBound = prim->WorldBound();
	}

	int IntersectPrimitiveTriangle(const Ray& ray, const Range& range, const TransformCache* cache, float& outtmin, Vector3& normal, Intersection& isect)
	{
		float tmin = range.tmax;
		int intersect = 0;
		int minTriIndex = -1;
		float minU, minV, minW;
		for(size_t i = 0; i < cache->prim->mesh->numIndices / 3; i++)
		{
			float u, v, w;
			float t;
			 
			int index[3];
			index[0] = i * 3;
			index[1] = i * 3 + 1;
			index[2] = i * 3 + 2;

			if(IntersectSegmentTriangle(ray, range, cache->worldPosCache, index, u, v, w, t) == 1)
			{
				if(t < tmin)
				{
					outtmin =tmin = t;
					intersect = 1;
					minTriIndex = i;
					minU = u;
					minV = v;
					minW = w;
				}
			}
		}

		if(intersect == 1)
		{
			Vector3 n0, n1, n2;

			n0 = cache->worldNormalCache[minTriIndex * 3];
			n1 = cache->worldNormalCache[minTriIndex * 3 + 1];
			n2 = cache->worldNormalCache[minTriIndex * 3 + 2];

			normal = Normalize(n0 * minU + n1 * minV + n2 * minW);
		}

		return intersect;
	}

	int Intersect(const Ray& ray, const Scene* scene, const TransformCache* cache, Intersection& isect)
	{
		float epsilon = 0.00001f;
		float tmin = FLT_MAX;		
		int intersect = 0;

		for(size_t i = 0 ; i < scene->primList.size(); i++)
		{
			const AABB& worldBound = cache[i].worldBound;
			Range newRange;
			if(IntersectRayAABB(ray, worldBound, newRange, epsilon) == 1  )
			{
				float triTmin = FLT_MAX ;
				Vector3 normal;
				if(IntersectPrimitiveTriangle(ray, newRange, &cache[i], triTmin, normal, isect) == 1 && triTmin < tmin)
				{
					tmin = triTmin;
					isect.prim = cache->prim;
					isect.p = GetPoint(ray, tmin);
					isect.n = normal;	
					intersect = 1;
				}
			}
		}
		
		return intersect;
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

	Vector3 Shade(const Scene* scene, const Ray& ray, const Intersection& isect)
	{
		Vector3 color = Vector3(0.f);
		for(int i = 0; i < scene->lightList.size(); i++)
		{
			Light* light = scene->lightList[i];

			switch(light->type)
			{
			case(Light::LT_POINT):
				{
					PointLight* ptLight = (PointLight*) (light);
					Vector3 l = ptLight->pos - isect.p;
					float distSquared = l.SquaredLength();
					l.Normalize();
					Vector3 wo = Normalize(-ray.direction);
					float nDotL = std::max(dot(isect.n, l), 0.f);
					color += ptLight->intensity * nDotL * ( BlinnBRDF(wo, l, isect.n, l, Vector3(0.04), 0.5) ) / distSquared;
					//color = isect.n ;
				}
				break;
			case(Light::LT_DIRECTION):
				{
					DirectionLight* dirLight = (DirectionLight*) (light);
					Vector3 l = dirLight->dir;
					l.Normalize();
					Vector3 wo = Normalize(-ray.direction);
					float nDotL = std::max(dot(isect.n, l), 0.f);
					color += dirLight->intensity * nDotL * ( BlinnBRDF(wo, l, isect.n, l, Vector3(0.04), 0.5) );
					//color = isect.n ;
				}
				break;
			}

		}

		return color;
	}

	void RendererRT::ScreenShot(const char* path, const Camera* camera, const Scene* scene)
	{
		unsigned char* imgBuf = new unsigned char[options.width * options.height * 4];
		TransformCache* transformCache = new TransformCache[scene->primList.size()];
		for(size_t i = 0; i < scene->primList.size(); i++)
		{
			BuildCache(camera, scene->primList[i], transformCache[i]);
		}

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

				if(Intersect(ray, scene, transformCache, isect))
				{
					Vector3 color = Shade(scene, ray, isect);
					SetColor(imgBuf, options.width, options.height, j, i, color);
				}
				else
				{
					SetColor(imgBuf, options.width, options.height, j, i, Vector3(0.5, 0.5, 0.5));
				}

			}
		}

		SaveTGA(path, imgBuf,  options.width, options.height);

		delete [] imgBuf;
		delete [] transformCache;
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