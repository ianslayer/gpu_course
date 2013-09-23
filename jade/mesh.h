#ifndef MESH_H
#define MESH_H

#include "../vector.h"
#include "../geometry.h"

struct VertexP3N3T2
{
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
};

class Mesh
{
public:

    Mesh();
    ~Mesh();

    VertexP3N3T2*   vertices;

    Vector3*        positionList;
    Vector3*        normalList;
    Vector2*        texcoordList;

    int*            indices;

    int             numVertices;
    int             numIndices;
};

#endif