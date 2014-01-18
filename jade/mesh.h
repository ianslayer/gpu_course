#ifndef MESH_H
#define MESH_H

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

    class Mesh : public RefCounted
    {
    public:

        Mesh();
        ~Mesh();

		std::string		name;

        VertexP3N3T2*   vertices;

        Vector3*        positionList;
        Vector3*        normalList;
        Vector2*        texcoordList;

        int*            indices;

        int             numVertices;
        int             numIndices;

        RefCountedPtr<HWVertexBuffer> vertexBuffer;
        RefCountedPtr<HWIndexBuffer>  indexBuffer;
		AABB			bound;
    };

	bool LoadFromObjMesh(const ObjMesh& objMesh, size_t geomIndex, RenderDevice* device, Mesh** outMesh );
	AABB ComputeBound(const Mesh& mesh);
}

#endif