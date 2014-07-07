#ifndef LIGHT_H
#define LIGHT_H
#include "refcount.h"
#include "../vector.h"
#include "color.h"
#include "../matrix.h"
#include "geometry.h"
#include "mesh.h"
#include "primitive.h"
#include "monte_carlo.h"

namespace jade
{
	class Light : public RefCounted
	{
	public:
		enum TYPE
		{
			LT_POINT = 1,
			LT_DIRECTION = 2,
			LT_GEOMETRY_AREA = 4,
			LT_SPHERE_AREA = 8,
		};

		Light(TYPE _type) : type(_type), power(0.f)
		{

		}

		const TYPE type;
		Vector3 power;
	};

	class PointLight : public Light
	{
	public:

		PointLight() : pos(0.f, 0.f, 10.f), intensity(1.f), radius(1.f), Light(LT_POINT) 
		{
			ComputePower();
		}
		PointLight(const Vector3& _pos, const Vector3 _intensity, float _radius) : pos(_pos), intensity(_intensity), radius(_radius), Light(LT_POINT) 
		{
			ComputePower();
		}

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
		void	  ComputePower()
		{
			power = 4.f * M_PI * intensity;
		}

		Vector3 pos;
		Vector3 intensity;
		float   radius;

	};

	class DirectionLight : public Light
	{
	public:

		DirectionLight() : dir(0, 0, 1), radiance(1.f), Light(LT_DIRECTION) {}
		DirectionLight(const Vector3& _dir, const Vector3 _radiance) : dir(_dir), radiance(_radiance), Light(LT_DIRECTION) {}
		Matrix4x4 ShadowViewMatrix() const;
		Matrix4x4 InvShadowViewMatrix() const;
		Matrix4x4 ShadowProjMatrix(const AABB& worldBound) const;

		void ComputePower(const AABB& sceneBound)
		{
			Sphere sceneBoundSphere = BoundSphere(sceneBound);
			power = radiance * M_PI * sceneBoundSphere.radius * sceneBoundSphere.radius;
		}

		Vector3 dir;
		Vector3 radiance;
	};

	class PrimitiveSampler
	{
	public:
		PrimitiveSampler() : prim(NULL), areaDistribution(NULL), sumArea(0.f)
		{
			
		}
		PrimitiveSampler(Primitive* _prim) : prim(_prim)
		{
			sumArea = 0;
			Matrix4x4 worldMatrix = prim->ModelMatrix();

				
			for(int i = 0; i < prim->mesh->numIndices / 3; i++)
			{
				int i0, i1, i2;
				i0 = prim->mesh->indices[i * 3];
				i1 = prim->mesh->indices[i * 3 + 1];
				i2 = prim->mesh->indices[i * 3 + 2];
					
				Vector3 p0 = TransformPoint(worldMatrix, prim->mesh->vertices[i0].position);
				Vector3 p1 = TransformPoint(worldMatrix, prim->mesh->vertices[i1].position);
				Vector3 p2 = TransformPoint(worldMatrix, prim->mesh->vertices[i2].position);
				float triArea = TriangleArea(p0, p1, p2);
				sumArea += triArea;
				areaList.push_back( triArea );
			}

			areaDistribution = new Distribution1D(&areaList[0], areaList.size());
		}

		~PrimitiveSampler()
		{
			delete areaDistribution;
		}

		Vector3 Sample(float u1, float u2, float u3, Vector3& normal) const
		{
			int tri = areaDistribution->SampleDiscrete(u3, NULL);

			Vector2 barycentricCoord = UniformSampleTriangle(u1, u2);
			
			float b1 = barycentricCoord.x;
			float b2 = barycentricCoord.y;
			float b3 = 1.f - b1 - b2;

			Vector3 p1, p2, p3, n1, n2, n3;
			int i0, i1, i2;
			i0 = prim->mesh->indices[tri * 3];
			i1 = prim->mesh->indices[tri * 3 + 1];
			i2 = prim->mesh->indices[tri * 3 + 2];

			Matrix4x4 worldMatrix = prim->ModelMatrix();

			p1 = prim->mesh->positionList[i0];
			p2 = prim->mesh->positionList[i1];
			p3 = prim->mesh->positionList[i2];

			n1 = prim->mesh->normalList[i0];
			n2 = prim->mesh->normalList[i1];
			n3 = prim->mesh->normalList[i2];

			normal = Normalize(TransformVector(worldMatrix, Normalize(b1 * n1 + b2 * n2 + b3 * n3)));

			return TransformPoint(worldMatrix, b1 * p1 + b2 * p2 + b3 * p3);

		}
		
		float Pdf(const Vector3& p, const Vector3& wi) const
		{
			Ray ray;
			ray.origin = p;
			ray.direction = wi;

			Range rayRange; //first get ray bound
			
			float pdf = 0;//(p - isectPoint).SquaredLength() / (abs(dot(isectNormal, -wi)) * sumArea);
		
			Matrix4x4 invModelTransform = prim->InvModelMatrix();
			Ray localRay = Transform(invModelTransform, ray);
			
			if(!IntersectRayAABB(localRay, prim->mesh->bound, rayRange, 0.0001))
				return 0.f;
			
			
			for(int i = 0; i < prim->mesh->numIndices / 3; i++)
			{
				float u, v, w, t;
				if(IntersectSegmentTriangle(localRay, rayRange, prim->mesh->vertices, &prim->mesh->indices[i*3], u, v, w, t))
				{
					int i0 = prim->mesh->indices[i*3];
					int i1 = prim->mesh->indices[i*3 + 1];
					int i2 = prim->mesh->indices[i*3 + 2];
					Vector3 localNormal = u * prim->mesh->normalList[i0] + v * prim->mesh->normalList[i1] + w * prim->mesh->normalList[i2] ;
					
					float triArea = areaList[i];
					
					Vector3 isectNormal = Normalize(TransformVector(prim->ModelMatrix(), Normalize(localNormal) ));
					Vector3 isectPoint = TransformPoint(prim->ModelMatrix(), GetPoint(localRay, t));
					
					pdf += (p - isectPoint).SquaredLength() / (abs(dot(isectNormal, -wi)) );
				}
			}
			
			/*
			if(!Intersect(*prim, ray, 1e-3f, mint, isectNormal) )
				return 0;
			*/
			
			
			pdf /= sumArea;
			
			if (isinf(pdf)) pdf = 0.f;	
			
			return pdf;
		}
		
		Primitive* prim;
		std::vector<float> areaList;
		Distribution1D* areaDistribution;
		float sumArea;
	};

	class GeomAreaLight : public Light
	{
	public:
		GeomAreaLight() : radiance(0.f), Light(LT_GEOMETRY_AREA), prim(0), sampler()
		{
			power = Vector3(0.f);
		}
		GeomAreaLight(const Vector3 _radiance, Primitive* _prim) : Light(LT_GEOMETRY_AREA), radiance(_radiance), prim(_prim), sampler(_prim)
		{
			prim->areaLight = this;
			ComputePower();
		}
		
		float Area()
		{
			return sampler.sumArea;
		}

		Vector3 L(const Vector3& n, const Vector3& w) const
		{
			return dot(n, w) > 0.f ? radiance : Vector3(0.f);
		}
		
		void ComputePower()
		{
			power = radiance * Area() * M_PI;
		}

		Vector3 radiance;
		PrimitiveSampler sampler;
		RefCountedPtr<Primitive> prim;

	};

	class SphereAreaLight : public Light
	{
	public:
		SphereAreaLight() : radiance(0.f), Light(LT_SPHERE_AREA) {}
		
		Vector3 pos;
		Vector3 radiance;
		float   radius;
	};
	

	inline bool IsDeltaLight(const Light& light)
	{
		return ((light.type & Light::LT_POINT) | (light.type & Light::LT_DIRECTION) )!= 0;
	}
	
}

#endif