#include "mesh.h"
#include "material.h"
#include "primitive.h"
#include "texture.h"
#include "../obj_mesh.h"
#include "../file_utility.h"

namespace jade
{

Primitive::Primitive()
    : mesh(0), translation(0.f), castShadow(false), receiveShadow(false)
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
    
Matrix4x4 Primitive:: InvModelMatrix() const
{
    Matrix4x4 modelMatrix = ModelMatrix();
	return InverseAffine(modelMatrix);
}


AABB Primitive::WorldBound() const
{
    Matrix4x4 modelMatrix = ModelMatrix();
	return Transform(modelMatrix, mesh->bound);
}


void LoadFromObjMesh(const ObjMesh& objMesh, RenderDevice* device, TextureManager* texManater, std::vector<Primitive*>& primList)
{
	Matrix4x4 identityMatrix = Identity4x4();
	Matrix2x2 texFlipMatrix = Matrix2x2(
		1.f, 0.f,
		0.f, -1.f
		);
	LoadFromObjMesh(objMesh, device, texManater, identityMatrix, texFlipMatrix, primList);
}

void LoadFromObjMesh(const ObjMesh& objMesh, RenderDevice* device, TextureManager* texManater, Matrix4x4 transform, Matrix2x2 texcoordMatrix, std::vector<Primitive* >& primList)
{
	for(size_t i = 0; i < objMesh.geomList.size(); i++)
	{
		Primitive* prim = NULL;

		if(LoadFromObjMesh(objMesh, i, device, texManater, transform, texcoordMatrix, &prim) )
			primList.push_back(prim);
	}
}


bool LoadFromObjMesh(const ObjMesh& objMesh, size_t geomIndex, RenderDevice* device, TextureManager* texManater, Matrix4x4 transform, Matrix2x2 texcoordMatrix, Primitive** outPrim)
{
	if(geomIndex > objMesh.geomList.size())
		return false;

	Mesh* mesh = new Mesh();
	LoadFromObjMesh(objMesh, geomIndex, device, transform, texcoordMatrix, &mesh);
	if(!*outPrim)
		*outPrim = new Primitive();

	Material* material = new Material();
	const ObjMesh::Material& mat = objMesh.matList[objMesh.geomList[geomIndex].matIndex];

	material->ambient = mat.ka;
	material->diffuse = mat.kd;
	material->specular = mat.ks;
	material->roughness = mat.ns;

	std::string folderPath = PathRemoveFileName(objMesh.path);

	if(!mat.mapKd.empty())
		material->diffuseMap = texManater->Load(folderPath + "\\" + mat.mapKd, true);
	if(!mat.mapKs.empty() )
		material->specularMap = texManater->Load( folderPath + "\\" + mat.mapKs );
	if(!mat.mapBump.empty())
		material->normalMap = texManater->Load( folderPath + "\\" + mat.mapBump );
	if(!mat.mapD.empty())
		material->dissolveMask = texManater->Load( folderPath + "\\" + mat.mapD );

	(*outPrim)->mesh = mesh;
	(*outPrim)->material = material;


	return true;
}

}