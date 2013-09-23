#include <stdio.h>
#include "../window.h"
#include "../glUtility.h"
#include "../vector.h"
#include "../matrix.h"
#include "../Timer.h"
#include "../camera.h"
#include "../image.h"
#include "../objMesh.h"

Window window;


class MyInputListener : public InputListener
{
public:
    explicit MyInputListener()
    {
    
    }

    virtual void OnMouseMove(const int x, const int y, const int deltaX, const int deltaY)
    {

    }

    virtual void OnMouseButton(const int x, const int y, const MouseButton button, const bool pressed)
    {

    }
    virtual void OnMouseWheel(const int x, const int y, const int scroll)
    {

    }

    virtual void OnKey(const unsigned int key, const bool pressed)
    {
  
    }

};

MyInputListener* inputListener = NULL;
ObjMesh mesh;

void InitResources()
{
    LoadObj("sponza.obj", &mesh);
}

void InputControl(double frameTime)
{

}

void InitGLState()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_TEXTURE_2D);
}

void Render(double frameTime)
{

    glClearColor(0.5, 0.5, 0.5, 1);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SwapBuffers(window.hdc);

}

void main()
{
    double frameTime = 16.0;

    FastTimer::Initialize();

    if(!InitWindow(800, 800, window))
        return;

    inputListener = new MyInputListener();
    SetInputListener(inputListener);

    if(!InitGL(window.hwnd, 4))
        return;
    InitGLState();

    InitResources();

    FastTimer timer;

    timer.Start();
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
            //if( GetMessage( &msg, NULL, 0, 0 ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        InputControl(frameTime);
        Render(frameTime);
    }
    timer.End();
    frameTime = timer.GetDurationInMillisecnds();
}