#include "mesh.h"
#include "material.h"
#include "primitive.h"
#include "texture.h"
#include "camera.h"
#include "../obj_mesh.h"
#include "../file_utility.h"

namespace jade
{

Primitive::Primitive()
    : mesh(0), translation(0.f), castShadow(false), receiveShadow(false), areaLight(0)
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
	return Inverse(modelMatrix);
}


AABB Primitive::WorldBound() const
{
    Matrix4x4 modelMatrix = ModelMatrix();
	return Transform(modelMatrix, mesh->bound);
}

int Intersect(const Primitive& prim, const Ray& ray, float epsilon, float& tmin, Vector3& isectNormal)
{
	Ray localRay;
	Range rayRange;
	Matrix4x4 invModelTransform = prim.InvModelMatrix();
	localRay = Transform(invModelTransform, ray);
	
	int testIntersect = IntersectRayAABB(ray, prim.WorldBound(), rayRange, epsilon);
	
	if( !IntersectRayAABB(localRay, prim.mesh->bound, rayRange, epsilon) )
		return 0;
	
	tmin = FLT_MAX;
	int isect = 0;
	float isectU, isectV, isectW;
	int isectTriIndex = -1;
	for(int i = 0; i < prim.mesh->numIndices / 3; i++)
	{
		float u, v, w, t;
		if(IntersectSegmentTriangle(localRay, rayRange, prim.mesh->vertices, &prim.mesh->indices[i*3], u, v, w, t))
		{
			if(t < tmin)
			{
				tmin = t;
				isectTriIndex = i * 3;
				isect = 1;
				isectU = u;
				isectV = v;
				isectW = w;
			}
		}
	}
	
	if(isect == 1)
	{
		int i0 = prim.mesh->indices[isectTriIndex];
		int i1 = prim.mesh->indices[isectTriIndex + 1];
		int i2 = prim.mesh->indices[isectTriIndex + 2];
		Vector3 localNormal = isectU * prim.mesh->normalList[i0] + isectV * prim.mesh->normalList[i1] + isectW * prim.mesh->normalList[i2] ;
		
		isectNormal = Normalize(TransformVector(prim.ModelMatrix(), Normalize(localNormal) ));
	}
	
	return isect;
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
	material->specularF0 = mat.ks;
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

Primitive* CreateCube(TextureManager* texManater, const Vector3& pos, const Vector3 radius, Mesh* cubeMesh)
{
	Primitive* prim = new Primitive();
	prim->mesh = cubeMesh;
	prim->translation = pos;
	prim->orientation = Matrix3x3(radius.x, 0, 0,
									0, radius.y, 0,
									0, 0, radius.z);

	prim->material = new Material();
	prim->material->diffuse = Vector3(0.3f);
	prim->material->specularF0 = Vector3(1.00,0.71,0.29);
	prim->material->roughness = 1024.f;
	
	prim->material->diffuseMap = texManater->Load("sys:white");

	return prim;
}

}