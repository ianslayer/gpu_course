#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "refcount.h"
#include "../matrix.h"
#include "../vector.h"
#include "geometry.h"
#include <vector>

class ObjMesh;

namespace jade
{
    class Mesh;
    class Material;
	class TextureManager;
	class RenderDevice;
	class Primitive;
	class Camera;
	class GeomAreaLight;
	class Ray;
	
    class Primitive : public RefCounted
    {
    public:
        Primitive();
        ~Primitive();

        Matrix4x4 ModelMatrix() const;
        Matrix4x4 InvModelMatrix() const;
        AABB	  WorldBound() const;

        RefCountedPtr<Mesh>			mesh;
        RefCountedPtr<Material>		material;

        Matrix3x3					orientation;
        Vector3						translation;
		
        bool                        castShadow;
        bool                        receiveShadow;

		GeomAreaLight*				areaLight;
    };

	int Intersect(const Primitive& prim, const Ray& ray, float epsilon, float& t, Vector3& isectNormal);
	
	bool LoadFromObjMesh(const ObjMesh& objMesh, size_t geomIndex, RenderDevice* device, TextureManager* texManater, Matrix4x4 transform, Matrix2x2 texcoordMatrix, Primitive** outPrim);
	void LoadFromObjMesh(const ObjMesh& objMesh, RenderDevice* device, TextureManager* texManater, Matrix4x4 transform, Matrix2x2 texcoordMatrix, std::vector<Primitive* >& primList);
	void LoadFromObjMesh(const ObjMesh& objMesh, RenderDevice* device, TextureManager* texManater, std::vector<Primitive*>& primList);

	Primitive* CreateCube(TextureManager* texManater, const Vector3& pos, const Vector3 radius, Mesh* cubeMesh);
}

#endif