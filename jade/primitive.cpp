#include "mesh.h"
#include "material.h"
#include "primitive.h"
#include "texture.h"
#include "../obj_mesh.h"
#include "../file_utility.h"

namespace jade
{

Primitive::Primitive()
    : mesh(0), translation(0.f)
{
    orientation.MakeIdentity();
}

Primitive::~Primitive()
{
}

Matrix4x4 Primitive::ModelMatrix() const
{
    Matrix4x4 modelMatrix(orientation[0][0], orientation[0][1], orientation[0][2], translation[0], 
                          orientation[1][0], orientation[1][1], orientation[1][2], translation[1], 
                          orientation[2][0], orientation[2][1], orientation[2][2], translation[2],
                          0, 0, 0, 1);

    return modelMatrix;
}

bool LoadFromObjMesh(const std::string& path, RenderDevice* device, TextureManager* texManater, std::vector<Primitive*>& primList)
{
	Matrix4x4 identityMatrix = Identity4x4();
	Matrix4x4 texFlipMatrix = Matrix4x4(
		1.f, 0.f, 0.f, 0.f,
		0.f, -1.f, 0.f, 1.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
		);
	return LoadFromObjMesh(path, device, texManater, identityMatrix, texFlipMatrix, primList);
}

bool LoadFromObjMesh(const std::string& path, RenderDevice* device, TextureManager* texManater, Matrix4x4 posMatrix, Matrix4x4 texcoordMatrix, std::vector<Primitive* >& primList)
{
	ObjMesh objMesh;

	bool success = objMesh.Load(path);

	std::string folderPath = PathRemoveFileName(objMesh.path);

	for(size_t i = 0; i < objMesh.geomList.size(); i++)
	{
		std::vector<ObjMesh::FusedVertex> vertices;
		std::vector<int> indices;

		objMesh.CreateVertexIndexBuffer(i, vertices, indices);

		for(size_t v = 0; v < vertices.size(); v++)
		{
			Vector4 pos =  posMatrix * Vector4(vertices[v].position.x, vertices[v].position.y, vertices[v].position.z, 1.f);
			vertices[v].position = Vector3(pos.x, pos.y, pos.z);

			Vector4 normal = posMatrix * Vector4(vertices[v].normal.x, vertices[v].normal.y, vertices[v].normal.z, 1.f);
			vertices[v].normal = Vector3(normal.x, normal.y, normal.z);


			Vector4 texcoord(vertices[v].texcoord.x, vertices[v].texcoord.y, 0.f, 1.f);
			texcoord = texcoordMatrix * texcoord;
			vertices[v].texcoord.x = texcoord.x;
			vertices[v].texcoord.y = texcoord.y;
		}

		HWVertexBuffer* vertexBuffer;
		HWIndexBuffer* indexBuffer;

		device->CreateVertexBuffer(sizeof(ObjMesh::FusedVertex) * vertices.size(), &vertices[0], &vertexBuffer);
		device->CreateIndexBuffer(sizeof(int) * indices.size(), &indices[0], &indexBuffer);

		Mesh* mesh = new Mesh();

		mesh->vertexBuffer = vertexBuffer;
		mesh->indexBuffer = indexBuffer;
		mesh->numVertices = vertices.size();
		mesh->numIndices = indices.size();

		mesh->positionList = new Vector3[vertices.size()];
		mesh->normalList = new Vector3[vertices.size()];
		mesh->texcoordList = new Vector2[vertices.size()];

		mesh->vertices = new VertexP3N3T2[vertices.size()];

		for(size_t v = 0; v < vertices.size(); v++)
		{
			mesh->vertices[v].position = mesh->positionList[v] = vertices[v].position;
			mesh->vertices[v].normal = mesh->normalList[v] = vertices[v].normal;
			mesh->vertices[v].texcoord = mesh->texcoordList[v] = vertices[v].texcoord;
		}

		Primitive* prim = new Primitive();
		Material* material = new Material();
		ObjMesh::Material& mat = objMesh.matList[objMesh.geomList[i].matIndex];

		material->ambient = mat.ka;
		material->diffuse = mat.kd;
		material->specular = mat.ks;
		material->roughness = mat.ns;

		if(!mat.mapKd.empty())
			material->diffuseMap = texManater->Load(folderPath + "\\" + mat.mapKd );
		if(!mat.mapKs.empty() )
			material->specularMap = texManater->Load(folderPath + "\\" + mat.mapKs );
		if(!mat.mapBump.empty())
			material->normalMap = texManater->Load(folderPath + "\\" + mat.mapBump);
		if(!mat.mapD.empty())
			material->dissolveMask = texManater->Load(folderPath + "\\" + mat.mapD);

		prim->mesh = mesh;
		prim->material = material;

		primList.push_back(prim);
	}


	return success;
}

}