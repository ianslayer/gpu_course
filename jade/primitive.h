#ifndef PRIMITIVE_H
#define PRIMITIVE_H

class Mesh;
class Material;

class Primitive
{
public:
    Primitive();
    ~Primitive();

    Mesh*       mesh;
    Material*   material;
};

bool LoadFromObjMesh(const char* path, Primitive** prims, int* numPrims);

#endif