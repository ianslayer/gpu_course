#include "mesh.h"
#include "primitive.h"
#include "../objMesh.h"
#include "../geometry.h"


Primitive::Primitive()
    : mesh(0), material(0)
{

}

Primitive::~Primitive()
{
    delete mesh;
    delete material;
}

bool LoadFromObjMesh(const char* path, Primitive** prims, int* numPrims)
{
    ObjMesh objMesh;

    bool success = LoadObj(path, &objMesh);

    *numPrims = objMesh.geomList.size();

    *prims = new Primitive[*numPrims];

    for(int i = 0; i < *numPrims; i++)
    {

    }


    return success;
}