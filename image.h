#ifndef IMAGE_H
#define IMAGE_H

#include "gl_utility.h"

bool LoadTGA(const char* path, unsigned char** imgBuffer, int* width, int* height);
bool LoadBMP(const char* path, unsigned char** imgBuffer, int* width, int* height);

void FreeImageBuffer(const unsigned char* imgBuffer);

#endif