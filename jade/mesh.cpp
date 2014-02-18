#include "mesh.h"
#include "../obj_mesh.h"

namespace jade
{

Mesh::Mesh()
    : positionList(0), normalList(0), tangentList(0), texcoordList(0), indices(0), numVertices(0), numIndices(0)
{

}

Mesh::~Mesh()
{
    delete[] vertices;
    delete[] positionList;
    delete[] normalList;
    delete[] tangentList;
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


	device->CreateIndexBuffer(sizeof(int) * indices.size(), &indices[0], &indexBuffer);

	if(*outMesh == NULL)
		*outMesh = new Mesh();


	(*outMesh)->indexBuffer = indexBuffer;
	(*outMesh)->numVertices = vertices.size();
	(*outMesh)->numIndices = indices.size();

	(*outMesh)->positionList = new Vector3[vertices.size()];
	(*outMesh)->normalList = new Vector3[vertices.size()];
    (*outMesh)->tangentList = new Vector4[vertices.size()];
	(*outMesh)->texcoordList = new Vector2[vertices.size()];

    (*outMesh)->vertices = new VertexP3N3T4T2[vertices.size()];
    
    (*outMesh)->indices = new int[indices.size()];

	for(size_t v = 0; v < vertices.size(); v++)
	{
		(*outMesh)->vertices[v].position  = (*outMesh)->positionList[v] = vertices[v].position;
		(*outMesh)->vertices[v].normal  = (*outMesh)->normalList[v] = vertices[v].normal;
		(*outMesh)->vertices[v].texcoord  = (*outMesh)->texcoordList[v] = vertices[v].texcoord;
	}
    
    for(size_t i = 0; i < indices.size(); i++)
    {
        (*outMesh)->indices[i] = indices[i];
    }
        
	(*outMesh)->bound = ComputeBound(**outMesh);
    CalculateTangentSpace( (*outMesh)->positionList, (*outMesh)->normalList, (*outMesh)->texcoordList, (*outMesh)->numVertices, (*outMesh)->indices, (*outMesh)->numIndices, (*outMesh)->tangentList);
    
    for(size_t v = 0; v < (*outMesh)->numVertices; v++)
    {
        (*outMesh)->vertices[v].tangent = (*outMesh)->tangentList[v];
    }
    
    
    HWVertexBuffer* vertexBuffer2;
	device->CreateVertexBuffer(sizeof(VertexP3N3T4T2) * (*outMesh)->numVertices, (*outMesh)->vertices, &vertexBuffer2);
    
    (*outMesh)->vertexBuffer = vertexBuffer2;
    
	return true;
}
    
void CalculateTangentSpace(const Vector3* posList, const Vector3* normalList, const Vector2* texcoord, int numVertices, const int* indices, int numIndices, Vector4* tangentList)
{
    Vector3* tan1 = new Vector3[numVertices * 2];
    Vector3* tan2 = tan1 + numVertices;
    
    memset(tan1, 0, numVertices * sizeof(Vector3) * 2);
    float pairity = 1.f;
    for(int i = 0; i < numIndices ; i+=3 )
    {
        int i1 = indices[i];
        int i2 = indices[i+1];
        int i3 = indices[i+2];
        const Vector3& p1 = posList[i1];
        const Vector3& p2 = posList[i2];
        const Vector3& p3 = posList[i3];
        
        const Vector2& w1 = texcoord[i1];
        const Vector2& w2 = texcoord[i2];
        const Vector2& w3 = texcoord[i3];
        
        Vector3 q1 = p2 - p1;
        Vector3 q2 = p3 - p1;
        
        float s1 = w2.x - w1.x;
        float t1 = w2.y - w1.y;
        
        float s2 = w3.x - w1.x;
        float t2 = w3.y - w1.y;
        
		pairity = cross(w2 - w1, w3 - w1) > 0? 1.f : -1.f;

        float r = 1.0f / (s1 * t2 - s2 * t1);
        Vector3 sdir = (t2 * q1 - t1 * q2) * r ;//* pairity;
        Vector3 tdir = (s1 * q2 - s2 * q1) * r ;

        
        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;
        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
        
    }
    
    for(int i = 0; i < numVertices; i++)
    {
        const Vector3& n = normalList[i];
        const Vector3& t = tan1[i];
        
        tangentList[i] = Vector4(Normalize( t - n * dot(n, t) ), 0 );
        tangentList[i].w = (dot( cross(n, t), tan2[i] ) < 0.f )? -1.0 : 1.0;
    }
    
    delete [] tan1;
        
}

}