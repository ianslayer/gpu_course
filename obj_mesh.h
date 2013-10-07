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
            for(i = 0; i < 4; i++)
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

		enum ILLUM
		{
			COLOR_ON_AMBIENT_OFF = 0,
			COLOR_ON_AMBIENT_ON = 1,
			HIGHLIGHT_ON = 2,
			REFLECTION_ON_RAY_TRACE_ON = 3,
			TRANSPARENCY = 4, //glass on reflection: ray trace on

		};

        std::string name;
        
        Vector4     kd;
        Vector4     ka;
        Vector4     ks;
        float		ns;
        int			illum;

        std::string mapKa;
        std::string mapKd;
        std::string mapD;
        std::string mapBump;
		std::string mapRefl;
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

    struct FusedVertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 texcoord;
    };


    bool Load(const std::string& path);
    void CreateVertexIndexBuffer(int geomIndex, std::vector<FusedVertex>& vertices, std::vector<int>& indices);

	std::string name;
	std::string path;

    std::vector<Vector3> posList;
    std::vector<Vector3> normalList;
    std::vector<Vector2> texcoordList;

    std::vector<Material> matList;
    std::vector<Geometry> geomList;
};



#endif