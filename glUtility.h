#ifndef GL_UTILITY_H
#define GL_UTILITY_H

#define NOMINMAX
#include "GL/gl3w.h"
#include "GL/wglext.h"
#include "vector.h"

struct MyVertex
{
    Vector3 position;
    Vector3 color;
};

bool InitGL(HWND hwnd, int msaaCount);
GLuint CompileShader(char* filename, GLuint shaderType);
GLuint CreateProgram(char* vsFilename, char* psFileName);

GLuint CreateCubeVertexBuffer();
GLuint CreateWireCubeIndexBuffer();

#endif