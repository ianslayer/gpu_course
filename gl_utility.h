#ifndef GL_UTILITY_H
#define GL_UTILITY_H

#define NOMINMAX
#include "GL/gl3w.h"


#ifdef _WIN32
#include "GL/wglext.h"
#endif

#include "vector.h"

#ifdef _WIN32
bool InitGL(HWND hwnd, int msaaCount);
#endif

GLuint CompileShader(char* filename, GLuint shaderType);
GLuint CreateProgram(char* vsFilename, char* psFileName);

struct MyVertex
{
    Vector3 position;
    Vector3 color;
};

GLuint CreateCubeVertexBuffer();
GLuint CreateWireCubeIndexBuffer();
GLuint CreateOctahedronVertexBuffer();
GLuint CreateWireOctahedronIndexBuffer();

struct SolidVertex
{
    Vector3 position;
    Vector3 normal;
    Vector3 color;
    Vector2 texcoord;
};

GLuint CreateSolidCubeVertexBuffer();
GLuint CreateSolidCubeIndexBuffer();

#endif