#ifndef OBJ_MESH_H
#define OBJ_MESH_H
#include <vector>
#include <string>
#include "vector.h"

class ObjMesh
{
public:
    class Vertex
    {
    public:
        Vertex() : vIndex(-1), nIndex(-1), tIndex(-1)
        {

        }

        bool HasNormal() { return nIndex != -1; }
        bool HasTexcoord() { return nIndex != -1; }

        int vIndex;
        int nIndex;
        int tIndex;
    };

    class Face
    {
    public:
        Vertex v[4];

        int NumVertices()
        {
            int i = 0;
            for(int i = 0; i < 4; i++)
            {
                if(v[i].vIndex == -1)
                    break;
            }
            return i;
        }
    };

    class Triangle
    {
    public:
        Vertex v[3];
    };

    class Material
    {
    public:
        std::string name;
        
        Vector4     kd;
        Vector4     ka;
        Vector4     ks;
        Vector4     Ns;
        float       illum;

        std::string mapKa;
        std::string mapKd;
        std::string mapD;
        std::string mapBump;
    };

    class Geometry
    {
    public:
        std::string             name;
        std::vector<Face>       faceList;
        std::vector<Triangle>   triangleList; //triangulated from face list
        int                     matIndex;
    };

    void Clear()
    {
        posList.clear();
        normalList.clear();
        texcoordList.clear();
        matList.clear();
        geomList.clear();
    }

    std::vector<Vector3> posList;
    std::vector<Vector3> normalList;
    std::vector<Vector2> texcoordList;

    std::vector<Material> matList;
    std::vector<Geometry> geomList;
};

bool LoadObj(const char* path, ObjMesh* mesh);

#endif