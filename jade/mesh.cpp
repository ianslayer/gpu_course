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

bool LoadFromObjMesh(const std::string& path, RenderDevice* device, std::vector<RefCountedPtr<Mesh> >& meshList)
{
    ObjMesh objMesh;

    bool success = objMesh.Load(path);

    for(size_t i = 0; i < objMesh.geomList.size(); i++)
    {
        std::vector<ObjMesh::FusedVertex> vertices;
        std::vector<int> indices;

        objMesh.CreateVertexIndexBuffer(i, vertices, indices);

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

        meshList.push_back(mesh);
    }


    return success;
}

}