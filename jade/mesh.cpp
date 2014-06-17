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
	return ComputeBound(_mesh.positionList, _mesh.numVertices);
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
		vertices[i].normal = Normalize(DiscardW(transform * Vector4(vertices[i].normal, 0.f) ));
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

Mesh* CreateMeshCube(RenderDevice* device)
{
	VertexP3N3T4T2 cube[8];
	cube[0].position = Vector3(1, 1, 1);
	cube[1].position = Vector3(1, 1, -1);
	cube[2].position = Vector3(1, -1, 1);
	cube[3].position = Vector3(1, -1, -1);
	cube[4].position = Vector3(-1, 1, 1);
	cube[5].position = Vector3(-1, 1, -1);
	cube[6].position = Vector3(-1, -1, 1);
	cube[7].position = Vector3(-1, -1, -1);


	Vector3 faceNormal[6] = 
	{
		Vector3(1, 0, 0),
		Vector3(0, 1, 0),
		Vector3(0, 0, 1),
		Vector3(-1, 0, 0),
		Vector3(0, -1, 0),
		Vector3(0, 0, -1)
	};

	VertexP3N3T4T2* cubeVertices = new VertexP3N3T4T2[24];

	//+x face
	cubeVertices[0] = cube[0]; 
	cubeVertices[1] = cube[2]; 
	cubeVertices[2] = cube[1]; 
	cubeVertices[3] = cube[3]; 

	cubeVertices[0].normal = faceNormal[0];
	cubeVertices[1].normal = faceNormal[0];
	cubeVertices[2].normal = faceNormal[0];
	cubeVertices[3].normal = faceNormal[0];

	cubeVertices[0].texcoord = Vector2(1, 1);
	cubeVertices[1].texcoord = Vector2(0, 1);
	cubeVertices[2].texcoord = Vector2(1, 0);
	cubeVertices[3].texcoord = Vector2(0, 0);

	//+y face
	cubeVertices[4] = cube[4]; 
	cubeVertices[5] = cube[0];
	cubeVertices[6] = cube[5];
	cubeVertices[7] = cube[1];

	cubeVertices[4].normal = faceNormal[1];
	cubeVertices[5].normal = faceNormal[1];
	cubeVertices[6].normal = faceNormal[1];
	cubeVertices[7].normal = faceNormal[1];

	cubeVertices[4].texcoord = Vector2(1, 1);
	cubeVertices[5].texcoord = Vector2(0, 1);
	cubeVertices[6].texcoord = Vector2(1, 0);
	cubeVertices[7].texcoord = Vector2(0, 0);

	//-x face
	cubeVertices[8] = cube[6];
	cubeVertices[9] = cube[4];
	cubeVertices[10] = cube[7];
	cubeVertices[11] = cube[5];

	cubeVertices[8].normal = faceNormal[3];
	cubeVertices[9].normal = faceNormal[3];
	cubeVertices[10].normal = faceNormal[3];
	cubeVertices[11].normal = faceNormal[3];

	cubeVertices[8].texcoord = Vector2(1, 1);
	cubeVertices[9].texcoord = Vector2(0, 1);
	cubeVertices[10].texcoord = Vector2(1, 0);
	cubeVertices[11].texcoord = Vector2(0, 0);

	//-y face
	cubeVertices[12] = cube[2];
	cubeVertices[13] = cube[6];
	cubeVertices[14] = cube[3];
	cubeVertices[15] = cube[7];

	cubeVertices[12].normal = faceNormal[4];
	cubeVertices[13].normal = faceNormal[4];
	cubeVertices[14].normal = faceNormal[4];
	cubeVertices[15].normal = faceNormal[4];

	cubeVertices[12].texcoord = Vector2(1, 1);
	cubeVertices[13].texcoord = Vector2(0, 1);
	cubeVertices[14].texcoord = Vector2(1, 0);
	cubeVertices[15].texcoord = Vector2(0, 0);

	//+z face
	cubeVertices[16] = cube[4];
	cubeVertices[17] = cube[6];
	cubeVertices[18] = cube[0];
	cubeVertices[19] = cube[2];

	cubeVertices[16].normal = faceNormal[2];
	cubeVertices[17].normal = faceNormal[2];
	cubeVertices[18].normal = faceNormal[2];
	cubeVertices[19].normal = faceNormal[2];

	cubeVertices[16].texcoord = Vector2(1, 1);
	cubeVertices[17].texcoord = Vector2(0, 1);
	cubeVertices[18].texcoord = Vector2(1, 0);
	cubeVertices[19].texcoord = Vector2(0, 0);

	//-z face
	cubeVertices[20] = cube[1];
	cubeVertices[21] = cube[3];
	cubeVertices[22] = cube[5];
	cubeVertices[23] = cube[7];

	cubeVertices[20].normal = faceNormal[5];
	cubeVertices[21].normal = faceNormal[5];
	cubeVertices[22].normal = faceNormal[5];
	cubeVertices[23].normal = faceNormal[5];

	cubeVertices[20].texcoord = Vector2(1, 1);
	cubeVertices[21].texcoord = Vector2(0, 1);
	cubeVertices[22].texcoord = Vector2(1, 0);
	cubeVertices[23].texcoord = Vector2(0, 0);

	Mesh* mesh = new Mesh();
	mesh->vertices = cubeVertices;
	mesh->numVertices = 24;
	
	mesh->positionList = new Vector3[24];
	mesh->normalList = new Vector3[24];
	mesh->tangentList = new Vector4[24];
	mesh->texcoordList = new Vector2[24];
	
	for(int i = 0; i < 24; i++)
	{
		mesh->positionList[i] = cubeVertices[i].position;
		mesh->normalList[i] = cubeVertices[i].normal;
		mesh->tangentList[i] = cubeVertices[i].tangent;
		mesh->texcoordList[i] = cubeVertices[i].texcoord;
	}

   int boxIndices[36] = 
   {

       //+x
       0, 1, 2,
       2, 1, 3,

	   //+y
	   4, 5, 6,
	   6, 5, 7,

       //-x
       8, 9, 10,
       10, 9, 11,

       //-y
       12, 13, 14,
       14, 13, 15,

       //+z
       16, 17, 18,
       18, 17, 19,

       //-z
       20, 21, 22,
       22, 21, 23
   };

   mesh->indices = new int[36];
   mesh->numIndices = 36;
   memcpy(mesh->indices, boxIndices, sizeof(int) * 36);

   CalculateTangentSpace( mesh->positionList, mesh->normalList, mesh->texcoordList, mesh->numVertices, mesh->indices, mesh->numIndices, mesh->tangentList);

	HWVertexBuffer* vertexBuffer;
	HWIndexBuffer* indexBuffer;

	device->CreateVertexBuffer(sizeof(VertexP3N3T4T2) * 24, cubeVertices, &vertexBuffer);
	device->CreateIndexBuffer(sizeof(int) * 36, mesh->indices, &indexBuffer);

	mesh->indexBuffer = indexBuffer;
	mesh->vertexBuffer = vertexBuffer;

	mesh->bound = ComputeBound(*mesh);
	//give ray tracer a little nudge
	Matrix4x4 transform =Scale(Vector3(1.01f) );
	mesh->bound = Transform(transform, mesh->bound);

	return mesh;

}

/*
Mesh* CreateMeshSphere(RenderDevice* device)
{

}
*/

}