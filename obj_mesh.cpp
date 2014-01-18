#define NOMINMAX 
#include "obj_mesh.h"
#include "string_utility.h"
#include <vector>
#include <unordered_map>
#include <shlwapi.h>
#include "file_utility.h"

enum OBJ_MAT_TOKEN
{
	MAT_UNKNOWN,
	NEW_MAT,
	ILLUM,
	KD,
	KA,
	KS,
	NS,
	MAP_KA,
	MAP_KD,
	MAP_KS,
	MAP_NS,
	MAP_D,
	MAP_BUMP,
	MAP_REFL,
};

OBJ_MAT_TOKEN GetMatDecl(const char* decl)
{
	if(!(stricmp(decl, "newmtl")))
		return NEW_MAT;
	else if(!(stricmp(decl, "illum")) )
		return ILLUM;
	else if(!(stricmp(decl, "Kd")) )
		return KD;
	else if(!(stricmp(decl, "Ka")) )
		return KA;
	else if(!(stricmp(decl, "Ks")))
		return KS;
	else if(!(stricmp(decl, "Ns")))
		return NS;
	else if(!(stricmp(decl, "map_Ka")))
		return MAP_KA;
	else if(!(stricmp(decl, "map_Kd")))
		return MAP_KD;
	else if(!stricmp(decl, "map_Ks"))
		return MAP_KS;
	else if(!stricmp(decl, "map_Ns"))
		return MAP_NS;
	else if(!(stricmp(decl, "map_d"))) //dissolve texture
		return MAP_D;
	else if(!(stricmp(decl, "map_bump")))
		return MAP_BUMP;
	else if(!(stricmp(decl, "refl")))
		return MAP_REFL;
	return MAT_UNKNOWN;
}


void LoadObjMaterial(const std::string& path, std::vector<ObjMesh::Material>& matList)
{

	int fileSize = 0;
	char* matStr = 0;
	void* matStrBuf = 0;
	fileSize = FileSize(path);

	matStr = new char[fileSize + 1];
	ReadFile(path, matStr, fileSize + 1);
	matStr[fileSize] = '\0';

	if(fileSize == 0)
		return;

	const char* currentLine = matStr;
	int strLen = (int) strlen(currentLine);
	int currentMatIndex = -1;
	while( 1 )
	{
		int ret = GetNextLine(currentLine, strLen);

		int nonBlank = 0;


		if( (nonBlank = GetFirstNonBlank(currentLine, ret)) >= 0)
		{
			char lineBuf[1024];
			

			const char* nonBlankLine = currentLine + nonBlank;

			memcpy(lineBuf, nonBlankLine, std::max(ret + nonBlank - 1, 0));
			lineBuf[ std::max(ret + nonBlank - 1, 0)] = 0;

			char decl[256];
			memset(decl, 0, sizeof(decl));
			sscanf(nonBlankLine, "%s ", decl);

			char mapType[256] = {0};
			char TexName[256] = {0};

			switch(GetMatDecl(decl))
			{
			case NEW_MAT:
				{
					matList.push_back(ObjMesh::Material() );
					currentMatIndex++;
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					char usemtl[16];
					char matName[256];
					sscanf(lineBuf, "%s %s", usemtl, matName);
					mat->name = matName;
				}
				break;
			case ILLUM:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					float illum = 0.f;
					char illumBuf[16];
					sscanf(lineBuf, "%s %f", illumBuf, &illum);
					mat->illum = illum;
				}
				break;
			case KD:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					float r,g, b;
					r = g = b = 0.f;
					char kd[16];
					sscanf(lineBuf, "%s %f %f %f", kd, &r, &g, &b);
					mat->kd = Vector3(r, g, b);
				}
				break;
			case KA:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					float r,g, b;
					r = g = b = 0.f;
					char ka[16];
					sscanf(lineBuf, "%s %f %f %f", ka, &r, &g, &b);
					mat->ka = Vector3(r, g, b);
				}
				break;
			case KS:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					float r,g, b;
					r = g = b = 0.f;
					char ks[16];
					sscanf(lineBuf, "%s %f %f %f", ks, &r, &g, &b);
					mat->ks = Vector3(r, g, b);
				}
				break;
			case NS:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					float ns = 0.f;
					char Ns[16];
					sscanf(lineBuf, "%s %f", Ns, &ns);
					mat->ns = ns;
				}
				break;
			case MAP_KA:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					sscanf(lineBuf, "%s %s", mapType, TexName);
					mat->mapKa = std::string(TexName);
					//printf("%s\n", mat->mapKa.c_str());
				}
				break;
			case MAP_KD:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					sscanf(lineBuf, "%s %s", mapType, TexName);
					mat->mapKd = std::string(TexName);
					//printf("%s\n", mat->mapKd.c_str());
				}
				break;
			case MAP_KS:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					sscanf(lineBuf, "%s %s", mapType, TexName);
					mat->mapKs = std::string(TexName);
				}
				break;
			case MAP_NS:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					sscanf(lineBuf, "%s %s", mapType, TexName);
					mat->mapNs = std::string(TexName);
				}
				break;
			case MAP_D:
				{
					//ObjMesh::ObjMaterial* mat = &(matList[currentMatIndex]);
					sscanf(lineBuf, "%s %s", mapType, TexName);
					//printf("%s\n", mat->mapD.c_str());					
				}
				break;
			case MAP_BUMP:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					sscanf(lineBuf, "%s %s", mapType, TexName);
					mat->mapBump = std::string(TexName);
					//printf("%s\n", mat->mapBump.c_str());
				}
				break;
			case MAP_REFL:
				{
					ObjMesh::Material* mat = &(matList[currentMatIndex]);
					sscanf(lineBuf, "%s %s", mapType, TexName);
					mat->mapRefl = std::string(TexName);
				}
			}
		}

		currentLine = currentLine + ret;
		strLen -= ret;

		if(strLen <= 0)
			break;
	}

	delete [] matStr;
}

enum OBJ_ELEM
{
    OBJ_UNKNOWN,
    OBJ_MATERIAL_FILE,
    OBJ_USE_MATERIAL,
    OBJ_VERTEX,
    OBJ_VERTEX_NORMAL,
    OBJ_TEXCOORD,
    OBJ_FACE
};

enum OBJ_TOKEN
{
    OBJ_TOK_UNKNOWN,
    OBJ_TOK_MATERIAL_FILE,  //matlib
    OBJ_TOK_USE_MATERIAL,   //usemtl
    OBJ_TOK_GEOMETRY,       //g
    OBJ_TOK_VERTEX,         //v
    OBJ_TOK_VERTEX_NORMAL,  //vn
    OBJ_TOK_TEXCOORD,       //vt
    OBJ_TOK_FACE,           //f
    OBJ_TOK_SMOOTH,         //s
    OBJ_TOK_COMMENT,        //#...
    OBJ_TOK_SLASH,          // /
    OBJ_TOK_FLOAT,          //float
    OBJ_TOK_INDEX           //integer
};

class Token
{
public:
    Token()
        : sourceStr(NULL), type(OBJ_TOK_UNKNOWN), count(0), next(NULL)
    {

    }
    const char* sourceStr;
    OBJ_TOKEN   type;
    //int         startIndex;
    int         count;
    Token*      next;
};



bool IsComment(char c)
{
    return (c=='#');
}

void ReadComment(const char** str)
{
    while(**str != '\n')
    {
        if(**str == '\0') return;
        (*str)++;
    }
    //(*str)++;
}


void ReadNumber(const char** str, Token* token)
{
    token->sourceStr = *str;
    int count = 0;
    bool isFloat = false;

    while(CharIsNumeric(**str) || **str == '.' || **str =='-')
    {
        if(**str =='.') isFloat = true;
        (*str)++;
        count++;
    }
    token->count = count;
    if(isFloat) token->type = OBJ_TOK_FLOAT;
    else token->type=OBJ_TOK_INDEX;
}

int ReadString(const char** str, Token* token)
{	
    token->sourceStr = *str;
    int count = 0;
    while(!CharIsBlank(**str)) //hack to deal with file name string
    {
        (*str)++;
        count++;
    }
    token->count = count;

    if(strncmp (token->sourceStr, "vn", 2) == 0)
    {
        token->type = OBJ_TOK_VERTEX_NORMAL;
        return 0;
    }
    else if(strncmp (token->sourceStr, "vt", 2) == 0)
    {
        token->type = OBJ_TOK_TEXCOORD;
        return 0;
    }	
    else if(strncmp (token->sourceStr, "v", 1) == 0 )
    {
        token->type = OBJ_TOK_VERTEX;
        return 0;
    }
    else if(strncmp (token->sourceStr, "f", 1) == 0)
    {
        token->type = OBJ_TOK_FACE;
        return 0;
    }
    else if(strncmp (token->sourceStr, "g", 1) == 0)
    {
        token->type = OBJ_TOK_GEOMETRY;
        return 0;
    }
    else if(strncmp (token->sourceStr, "s", 1) == 0)
    {
        token->type = OBJ_TOK_SMOOTH;
        return 0;
    }
    else if(strncmp (token->sourceStr, "mtllib", 6) == 0)
    {
        token->type = OBJ_TOK_MATERIAL_FILE;
        return 0;
    }
    else if(strncmp (token->sourceStr, "usemtl", 6) == 0)
    {
        token->type = OBJ_TOK_USE_MATERIAL;
        return 0;
    }

    token->type = OBJ_TOK_UNKNOWN;
    return 1;
}


int Tokenize(const char* objStr, size_t strLen, std::vector<Token>& tokens)
{
    const char* currentStr = objStr;
    Token token;
    while(1)
    {
        if(*currentStr == 0) 
            for(int i = tokens.size() - 1; i > 0; i--)
            {	
                tokens[i-1].next = &tokens[i];
                return 0;
            }
            while(CharIsBlank(*currentStr)) currentStr++;

            if(IsComment(*currentStr)) { ReadComment(&currentStr); }
            else if(CharIsAlpha(*currentStr) || *currentStr == '_') { ReadString(&currentStr, &token); tokens.push_back(token); }
            else if(CharIsNumeric(*currentStr) || *currentStr == '-' ) { ReadNumber(&currentStr, &token); tokens.push_back(token); }
            else if(*currentStr == '/') { token.sourceStr = currentStr; token.count = 1; token.type = OBJ_TOK_SLASH; tokens.push_back(token); currentStr++; }
            else if(*currentStr == 0){ 
                for(int i = tokens.size() - 1; i > 0; i--)
                {	
                    tokens[i-1].next = &tokens[i];
                }
                return 0;
            }
            else { printf("unknown token\n"); return 1; }

            //return 0;
    }
    return 1;
}

int ParseVertex(const Token* const token, Token** nextToken, Vector3* v)
{
    *nextToken = token->next;
    int i = 0;
//    char strBuffer[256] ;
    for(; i < 3 && *nextToken && (*nextToken)->type == OBJ_TOK_FLOAT; i++)
    {
        //memcpy(strBuffer, (*nextToken)->sourceStr, std::min((*nextToken)->count, 255 ));
        //strBuffer[std::min((*nextToken)->count, 255 )] = (char)0;
        (*v)[i] = (float)atof((*nextToken)->sourceStr);
        *nextToken = (*nextToken)->next;
    }
    if(i < 3)
        return 1;
    return 0;
}

int ParseTexcoord(const Token* const token, Token** nextToken, Vector2* texcoord)
{
    *nextToken = token->next;
    int i = 0; 
    Vector3 texcoord3D;
    for(; i < 3 && *nextToken && (*nextToken)->type == OBJ_TOK_FLOAT; i++)
    {
        texcoord3D[i] = (float)atof((*nextToken)->sourceStr);
        *nextToken = (*nextToken)->next;
    }

    texcoord->x = texcoord3D.x;
    texcoord->y = texcoord3D.y;
    return 0;   
}

int ParseFaceVertex(Token* const token, Token** nextToken, ObjMesh::Face* poly)
{
    *nextToken = token->next;
    int i = 0;

    for(; *nextToken && (*nextToken)->type == OBJ_TOK_INDEX ; i++) 
    {
        if(i >= 4)
            return 1;

        poly->v[i].vIndex = poly->v[i].nIndex = poly->v[i].tIndex = -1;

        poly->v[i].vIndex = atoi( (*nextToken)->sourceStr );
        *nextToken = (*nextToken)->next;

        if( (*nextToken) == NULL || (*nextToken)->type != OBJ_TOK_SLASH)
            continue;

        *nextToken = (*nextToken)->next;
        if( (*nextToken)->type == OBJ_TOK_INDEX )
        {
            poly->v[i].tIndex = atoi((*nextToken)->sourceStr);
            *nextToken = (*nextToken)->next;
        }

        if((*nextToken) == NULL || (*nextToken)->type != OBJ_TOK_SLASH)
            continue;

        *nextToken = (*nextToken)->next;

        if( (*nextToken)->type == OBJ_TOK_INDEX )
        {
            poly->v[i].nIndex = atoi( (*nextToken)->sourceStr );
            *nextToken = (*nextToken)->next;
        }

    }
    return 0;
}

int Parse(std::vector<Token>& tokens, ObjMesh* mesh)
{
    if(tokens.size() == 0)
        return 1;

    mesh->Clear();

    int numParsedToken = 0;
    Token* parseToken = &tokens[0];
    Token* nextToken = NULL;

    ObjMesh::Geometry* currentSegment = NULL;

    while(parseToken)
    {
        switch(parseToken->type)
        {

        case OBJ_TOK_VERTEX:
            {
                Vector3 v;
                if(ParseVertex(parseToken, &nextToken, &v) ) 
                    return 1;
                mesh->posList.push_back(v);
                parseToken = nextToken;
                break;
            }
        case OBJ_TOK_VERTEX_NORMAL:
            {
                Vector3 vn;
                if(ParseVertex(parseToken, &nextToken, &vn) ) 
                    return 1;
                mesh->normalList.push_back(vn);
                parseToken = nextToken;                
            }
            break;
        case OBJ_TOK_TEXCOORD:
            {
                Vector2 xtexcoord;
                if(ParseTexcoord(parseToken, &nextToken, &xtexcoord))
                    return 1;
                mesh->texcoordList.push_back(xtexcoord);
                parseToken = nextToken;
            }
            break;

        case OBJ_TOK_FACE:
            {
                ObjMesh::Face poly;
                if(ParseFaceVertex(parseToken, &nextToken, &poly)) 
                    return 1;
                currentSegment->faceList.push_back(poly);
                parseToken = nextToken;
            }
            break;
        case OBJ_TOK_GEOMETRY:
            {
                parseToken = parseToken->next;
                mesh->geomList.push_back(ObjMesh::Geometry() );
                currentSegment = &(*mesh->geomList.rbegin());
                currentSegment->name = std::string(parseToken->sourceStr, parseToken->count);
                parseToken = parseToken->next;
            }
            break;
        case OBJ_TOK_MATERIAL_FILE:
            {
                parseToken = parseToken->next;
                std::string fileName(parseToken->sourceStr, parseToken->count);
				std::string folder = PathRemoveFileName(mesh->path);
                LoadObjMaterial(folder + "\\" + fileName, mesh->matList);
                parseToken = parseToken->next;
            }
            break;
        case OBJ_TOK_USE_MATERIAL:
            {
                parseToken = parseToken->next; //parseToken should be material name
                std::string matName(parseToken->sourceStr, parseToken->count);
                int matIndex = 0;
                for(size_t i = 0; i < mesh->matList.size(); i++)
                {
                    if(mesh->matList[i].name == matName )
                    {
                        matIndex = i;
                        break;
                    }
                }
                if(currentSegment == NULL)
                {
                    mesh->geomList.push_back(ObjMesh::Geometry() );
                    currentSegment = &(*mesh->geomList.rbegin());
                }
                currentSegment->matIndex = matIndex;
                parseToken = parseToken->next;
            }
            break;
        default:
            parseToken = parseToken->next;
            //printf("obj mesh parse failed\n");
            //return 1;
            break;
        }
    }

    printf("obj mesh parse success\n");
    return 0;
}

int Triangulate(ObjMesh* mesh)
{
    for(size_t i = 0; i < mesh->geomList.size(); i++)
    {
        ObjMesh::Geometry* segment = &(mesh->geomList[i]);
        for(size_t j = 0 ; j < segment->faceList.size(); j++)
        {
            ObjMesh::Face* poly = &(segment->faceList[j]);
            if(poly->NumVertices() > 3)
            {
                //should put real triangluation code here
                ObjMesh::Triangle t1, t2;
                t1.v[0] = poly->v[0];
                t1.v[1] = poly->v[1];
                t1.v[2] = poly->v[2];

                t2.v[0] = poly->v[2];
                t2.v[1] = poly->v[3];
                t2.v[2] = poly->v[0];

                segment->triangleList.push_back(t1);
                segment->triangleList.push_back(t2);
            }
            else
            {
                ObjMesh::Triangle triangle;
                triangle.v[0] = poly->v[0];
                triangle.v[1] = poly->v[1];
                triangle.v[2] = poly->v[2];
                segment->triangleList.push_back(triangle);
            }
        }
    }

    return 0;
}

bool LoadObj(const char* objFileStr, int fileLength, ObjMesh* mesh)
{
    std::vector<Token> tokens;

    if(Tokenize(objFileStr, fileLength, tokens) ) return false;
    if(Parse(tokens, mesh) ) return false;
    if(Triangulate(mesh) ) return false;

    return true;
}

bool ObjMesh::Load(const std::string& path)
{
	int objFileSize = FileSize(path);
    char* strBuf = new char[objFileSize + 1];
	void* fileBuf = strBuf;
    int fileLen = ReadFile(path, fileBuf, objFileSize + 1 );
    strBuf[fileLen] = '\0';

	this->path = NormalizePath(WorkingDir() + "\\" + path);
	this->name = PathRemoveFolder(path);
	std::string folder = PathRemoveFileName(this->path);

    bool success = LoadObj(strBuf, fileLen, this);

    delete [] strBuf;

    return success;
}

struct VertexHash
{
    size_t operator() (const ObjMesh::FusedVertex& vert) const
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

struct VertexEqual
{
    bool operator () (const ObjMesh::FusedVertex& vert0, const ObjMesh::FusedVertex& vert1) const
    {
        return (vert0.position == vert1.position) && (vert0.normal == vert1.normal) && (vert0.texcoord == vert1.texcoord);
    }
};

void ObjMesh::CreateVertexIndexBuffer(int geomIndex, std::vector<ObjMesh::FusedVertex>& vertices, std::vector<int>& indices) const
{
    std::unordered_map<FusedVertex, int, VertexHash, VertexEqual> vertIndexTable;
    int uniqueIndex = 0;

    for(size_t t = 0; t < geomList[geomIndex].triangleList.size(); t++)
    {
        int triIndices[3];

        Triangle tri = geomList[geomIndex].triangleList[t];
        for(int v = 0; v < 3; v++)
        {
            Vertex vert = tri.v[v];
            FusedVertex vertex;
            memset(&vertex, 0, sizeof(vertex));

            vertex.position = posList[vert.vIndex - 1];

            if( vert.HasNormal() )
            {
                vertex.normal = normalList[vert.nIndex - 1];
            }

            if( vert.HasTexcoord() )
            {
                vertex.texcoord = texcoordList[vert.tIndex - 1];
            }
            std::unordered_map<FusedVertex, int>::iterator indexIter = vertIndexTable.find(vertex);
      
            if(indexIter == vertIndexTable.end() )
            {
                vertIndexTable[vertex] = uniqueIndex;
                vertices.push_back(vertex);
                triIndices[v] = uniqueIndex;
                uniqueIndex++;
            }
            else
            {
                triIndices[v] = indexIter->second;
            }
        }

        indices.push_back(triIndices[0]);
        indices.push_back(triIndices[1]);
        indices.push_back(triIndices[2]);
    }
}