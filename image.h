#ifndef JOG_IMAGE_H
#define JOG_IMAGE_H

#include "GL/gl3w.h"

bool LoadTGA(const char* path, const unsigned char** imgBuffer, int* width, int* height);
bool LoadBMP(const char* path, const unsigned char** imgBuffer, int* width, int* height);

GLuint CreateGLImage(const unsigned char* imgBuffer, int width, int height);
GLuint CreateMipmapGLImage(const unsigned char* imgBuffer, int width, int height);

void FreeImageBuffer(const unsigned char* imgBuffer);

#endif