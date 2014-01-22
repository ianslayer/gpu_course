#include "mesh.h"
#include "../obj_mesh.h"

namespace jade
{

Mesh::Mesh()
    : vertices(0), positionList(0), normalList(0), texcoordList(0), indices(0), numVertices(0), numIndices(0)
{

}

Mesh::~Mesh()
{
    delete[] vertices;
    delete[] positionList;
    delete[] normalList;
    delete[] texcoordList;
    delete[] indices;
}

AABB ComputeBound(const Mesh& _mesh)
{
	Vector3 minBound = Vector3(FLT_MAX);
	Vector3 maxBound = Vector3(-FLT_MAX);
	AABB bound;

	for(int i = 0; i < _mesh.numVertices; i++)
	{
		minBound = Min(minBound, _mesh.positionList[i]);
		maxBound = Max(maxBound, _mesh.positionList[i]);
	}

	bound.center = (minBound + maxBound) / 2.f;
	bound.radius = (maxBound - minBound) / 2.f;

	return bound;
}

bool LoadFromObjMesh(const ObjMesh& objMesh, size_t geomIndex, RenderDevice* device, const Matrix4x4& transform, const Matrix2x2 texMatrix, Mesh** outMesh )
{
	//assert(geomIndex < objMesh.geomList.size());

	if(geomIndex > objMesh.geomList.size())
		return false;

	std::vector<ObjMesh::FusedVertex> vertices;
	std::vector<int> indices;

	objMesh.CreateVertexIndexBuffer(geomIndex, vertices, indices);

	for(size_t i= 0 ; i < vertices.size(); i++)
	{
		vertices[i].position = DivideW(transform * Vector4(vertices[i].position, 1.f));
		vertices[i].normal = DiscardW(transform * Vector4(vertices[i].normal, 0.f) );
		vertices[i].texcoord = texMatrix * vertices[i].texcoord;
	}

	HWVertexBuffer* vertexBuffer;
	HWIndexBuffer* indexBuffer;

	device->CreateVertexBuffer(sizeof(ObjMesh::FusedVertex) * vertices.size(), &vertices[0], &vertexBuffer);
	device->CreateIndexBuffer(sizeof(int) * indices.size(), &indices[0], &indexBuffer);

	if(*outMesh == NULL)
		*outMesh = new Mesh();

	(*outMesh)->vertexBuffer = vertexBuffer;
	(*outMesh)->indexBuffer = indexBuffer;
	(*outMesh)->numVertices = vertices.size();
	(*outMesh)->numIndices = indices.size();

	(*outMesh)->positionList = new Vector3[vertices.size()];
	(*outMesh)->normalList = new Vector3[vertices.size()];
	(*outMesh)->texcoordList = new Vector2[vertices.size()];

	(*outMesh)->vertices = new VertexP3N3T2[vertices.size()];

	(*outMesh)->bound = ComputeBound(**outMesh);

	for(size_t v = 0; v < vertices.size(); v++)
	{
		(*outMesh)->vertices[v].position = (*outMesh)->positionList[v] = vertices[v].position;
		(*outMesh)->vertices[v].normal = (*outMesh)->normalList[v] = vertices[v].normal;
		(*outMesh)->vertices[v].texcoord = (*outMesh)->texcoordList[v] = vertices[v].texcoord;
	}

	return true;
}

}