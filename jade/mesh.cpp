#include "mesh.h"

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