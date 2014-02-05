#ifndef MESH_H
#define MESH_H

#include "../matrix.h"
#include "../vector.h"
#include "../geometry.h"
#include "render_device.h"
#include "refcount.h"
#include <string>

class ObjMesh;

namespace jade
{
    struct VertexP3N3T2
    {
        Vector3 position;
        Vector3 normal;
        Vector2 texcoord;
    };
    
    struct VertexP3N3T4T2
    {
        Vector3 position;
        Vector3 normal;
        Vector4 tangent;
        Vector2 texcoord;
    };

    class Mesh : public RefCounted
    {
    public:

        Mesh();
        ~Mesh();

		std::string		name;

        VertexP3N3T2*   vertices;
        VertexP3N3T4T2* vertices2;
        
        Vector3*        positionList;
        Vector3*        normalList;
        Vector4*        tangentList;
        Vector2*        texcoordList;

        int*            indices;

        int             numVertices;
        int             numIndices;

        RefCountedPtr<HWVertexBuffer> vertexBuffer;
        RefCountedPtr<HWVertexBuffer> vertexBuffer2;
        RefCountedPtr<HWIndexBuffer>  indexBuffer;
		AABB			bound;
    };

	bool LoadFromObjMesh(const ObjMesh& objMesh, size_t geomIndex, RenderDevice* device, const Matrix4x4& transform, const Matrix2x2 texMatrix, Mesh** outMesh );
	AABB ComputeBound(const Mesh& mesh);
	void TransformMeshVertices(Mesh& mesh, const Matrix4x4& transform);
    
    void CalculateTangentSpace(const Vector3* posList, const Vector3* normalList, const Vector2* texcoord, int numVertices, const int* indices, int numIndices, Vector4* tangentList);
}

#endif