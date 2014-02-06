#ifndef GL_UTILITY_H
#define GL_UTILITY_H

#define NOMINMAX

#ifdef _WIN32
	 #include "GL/gl3w.h"
    #include "GL/wglext.h"
   
#elif defined(__APPLE__)
#define ESSENTIAL_GL_PRACTICES_SUPPORT_GL3 1
    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>
    #include <OpenGL/OpenGL.h>
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