#ifndef IMAGE_H
#define IMAGE_H

#include "gl_utility.h"

bool LoadTGA(const char* path, unsigned char** imgBuffer, int* width, int* height);
bool LoadBMP(const char* path, unsigned char** imgBuffer, int* width, int* height);

GLuint GenerateWhiteTexture();
GLuint GenerateColorTexture(float r, float g, float b, float a); //generate a single color, single pixel texture;
GLuint CreateGLImage(const unsigned char* imgBuffer, int width, int height);
GLuint CreateMipmapGLImage(const unsigned char* imgBuffer, int width, int height);

void FreeImageBuffer(const unsigned char* imgBuffer);

#endif