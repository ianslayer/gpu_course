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
		Material() : ka(0.f), kd(0.f), ks(0.f), ns(0.f)
		{}

        std::string name;
     
        Vector3     ka;   
        Vector3     kd;
        Vector3     ks;
        float		ns;


        int			illum;
		/*
		 Illumination    Properties that are turned on in the 
		 model           Property Editor
 
		 0		Color on and Ambient off
		 1		Color on and Ambient on
		 2		Highlight on
		 3		Reflection on and Ray trace on
		 4		Transparency: Glass on
 				Reflection: Ray trace on
		 5		Reflection: Fresnel on and Ray trace on
		 6		Transparency: Refraction on
 				Reflection: Fresnel off and Ray trace on
		 7		Transparency: Refraction on
 				Reflection: Fresnel on and Ray trace on
		 8		Reflection on and Ray trace off
		 9		Transparency: Glass on
 				Reflection: Ray trace off
		 10		Casts shadows onto invisible surfaces
		*/

        std::string mapKa;
        std::string mapKd;
		std::string mapKs;
		std::string mapNs;
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
    void CreateVertexIndexBuffer(int geomIndex, std::vector<FusedVertex>& vertices, std::vector<int>& indices) const;

	std::string name;
	std::string path;

    std::vector<Vector3> posList;
    std::vector<Vector3> normalList;
    std::vector<Vector2> texcoordList;

    std::vector<Material> matList;
    std::vector<Geometry> geomList;
};

template<class VertType>
class VertexHash
{
public:
    size_t operator() (const VertType& vert) const
    {
        const unsigned int magic1 = 0x8da6b343;
        const unsigned int magic2 = 0xd8163841;
        const unsigned int magic3 = 0xfa769893;
        const unsigned int magic4 = 0xea489237;
        const unsigned int magic5 = 0xab389235;
        const unsigned int magic6 = 0xaf387933;
        const unsigned int magic7 = 0xce339897;
        const unsigned int magic8 = 0xad390233;
        
        return (unsigned long) vert.position.x * magic1 + vert.position.y * magic2 + vert.position.z * magic3 +
        vert.normal.x * magic4 + vert.normal.y * magic5 + vert.normal.z * magic6 +
        vert.texcoord.x * magic7 + vert.texcoord.y * magic8;
    }
};

template<class VertType>
class VertexEqual
{
public:
    bool operator () (const VertType& vert0, const VertType& vert1) const
    {
        return (vert0.position == vert1.position) && (vert0.normal == vert1.normal) && (vert0.texcoord == vert1.texcoord);
    }
};

#endif