#ifndef WINDOW_H
#define WINDOW_H

#define NOMINMAX
#include <windows.h>
struct Window
{
    HWND hwnd;
    HDC hdc;
    int width;
    int height;
};

bool InitWindow(int width, int height, Window& window);

#endif