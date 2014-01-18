#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "refcount.h"
#include "../matrix.h"
#include "../vector.h"
#include <vector>

class ObjMesh;

namespace jade
{
    class Mesh;
    class Material;
	class TextureManager;
	class RenderDevice;

    class Primitive : public RefCounted
    {
    public:
        Primitive();
        ~Primitive();

        Matrix4x4 ModelMatrix() const;

        RefCountedPtr<Mesh>			mesh;
        RefCountedPtr<Material>		material;

        Matrix3x3					orientation;
        Vector3						translation;
    };

	bool LoadFromObjMesh(const ObjMesh& objMesh, size_t geomIndex, RenderDevice* device, TextureManager* texManater, Matrix4x4 posMatrix, Matrix4x4 texcoordMatrix, Primitive** outPrim);
	bool LoadFromObjMesh(const ObjMesh& objMesh, RenderDevice* device, TextureManager* texManater, Matrix4x4 posMatrix, Matrix4x4 texcoordMatrix, std::vector<Primitive* >& primList);

	bool LoadFromObjMesh(const std::string& path, RenderDevice* device, TextureManager* texManater, std::vector<Primitive*>& primList);
	bool LoadFromObjMesh(const std::string& path, RenderDevice* device, TextureManager* texManater, Matrix4x4 posMatrix, Matrix4x4 texcoordMatrix, std::vector<Primitive* >& primList);
}

#endif