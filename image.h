#ifndef IMAGE_H
#define IMAGE_H

#include "GL/gl3w.h"

bool LoadTGA(const char* path, unsigned char** imgBuffer, int* width, int* height);
bool LoadBMP(const char* path, unsigned char** imgBuffer, int* width, int* height);

GLuint GenerateWhiteTexture();
GLuint CreateGLImage(const unsigned char* imgBuffer, int width, int height);
GLuint CreateMipmapGLImage(const unsigned char* imgBuffer, int width, int height);

void FreeImageBuffer(const unsigned char* imgBuffer);

#endif