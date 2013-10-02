#include <stdio.h>
#include "../window.h"
#include "../gl_utility.h"
#include "../vector.h"
#include "../matrix.h"
#include "../Timer.h"
#include "../camera.h"
#include "../image.h"
#include "../obj_mesh.h"
#include "mesh.h"
#include "primitive.h"
#include "renderer_gl.h"
#include "camera.h"

Window window;
jade::RenderDevice* device;
jade::RendererGL* rendererGL;
jade::Camera camera;

float cameraMoveSpeed = 2.f;
float cameraTurnSpeed = 1.f;

class MyInputListener : public InputListener
{
public:
    explicit MyInputListener()
    {
        forward = false;
        backward = false;
        right = false;
        left = false;

        ClearState();
        mouseControl =false;
    }

    virtual void OnMouseMove(const int x, const int y, const int deltaX, const int deltaY)
    {
        rx += deltaX;
        ry += deltaY;
    }

    virtual void OnMouseButton(const int x, const int y, const MouseButton button, const bool pressed)
    {
        if(pressed && button == MOUSE_RIGHT)
        {
            mouseControl = true;
        }
        else
        {
            mouseControl = false;
        }
    }
    virtual void OnMouseWheel(const int x, const int y, const int scroll)
    {
        cameraMoveSpeed += scroll;
        cameraMoveSpeed = std::max(1.f, cameraMoveSpeed );
    }

    virtual void OnKey(const unsigned int key, const bool pressed)
    {
        if(key == KEY_W)
        {
            forward = pressed;
        }
        if( key == KEY_S)
        {
            backward = pressed;
        }
        if( key == KEY_A)
        {
            left = pressed;
        }
        if(key == KEY_D)
        {
            right = pressed;
        }
    }

    void ClearState()
    {
        rx = 0;
        ry = 0;
    }

    bool forward;
    bool backward;
    bool right;
    bool left;

    bool mouseControl;
    float rx;
    float ry;
};

MyInputListener* inputListener = NULL;

void InputControl(double frameTime)
{

    if(inputListener->forward)
    {
        camera.position += camera.lookat * cameraMoveSpeed * frameTime / 1000.f;
    }
    if(inputListener->backward)
    {
        camera.position -= camera.lookat * cameraMoveSpeed * frameTime / 1000.f;
    }
    if(inputListener->right)
    {
        camera.position += camera.right * cameraMoveSpeed * frameTime / 1000.f;
    }
    if(inputListener->left)
    {
        camera.position -= camera.right * cameraMoveSpeed * frameTime / 1000.f;
    }

    if(inputListener->mouseControl)
    {
        Matrix3x3 head = Matrix3x3::RotateAxis(camera.up, -inputListener->rx * cameraTurnSpeed * frameTime / 1000.f);
        camera.lookat = head * camera.lookat;
        camera.right = head *  camera.right;

        Matrix3x3 pitch = Matrix3x3::RotateAxis(camera.right, -inputListener->ry * cameraTurnSpeed * frameTime / 1000.f);
        camera.lookat = pitch * camera.lookat;
        camera.up = pitch *  camera.up;

        //make sure right is horizontal
        camera.right.z = 0;
        camera.right.Normalize();
        camera.lookat = cross(camera.up, camera.right);
        camera.up = cross(camera.right, camera.lookat);
    }

    inputListener->ClearState();
}

void Init()
{
    if(!InitWindow(800, 800, window))
        return;

    jade::RenderDeviceSetting deviceSetting;
    jade::InitRenderDevice(&window, &deviceSetting, &device);
    jade::InitRendererGL(device, &rendererGL);    

    inputListener = new MyInputListener();
    SetInputListener(inputListener);
}

void Shutdown()
{
    jade::ShutdownRendererGL(rendererGL);
    jade::ShutdownRenderDevice(device);
}

std::vector<jade::RefCountedPtr<jade::Mesh> > meshList;
std::vector<jade::RefCountedPtr<jade::Primitive> > primitiveList;

void LoadResources()
{
    jade::LoadFromObjMesh("data/db5/db5.obj", device, meshList);
    
    for(size_t i = 0; i < meshList.size(); i++)
    {
        jade::Primitive* prim = new jade::Primitive();
        prim->mesh = meshList[i];

        primitiveList.push_back(prim);
    }

}

void UnloadResources()
{

}

void Render(double frameTime)
{

    glClearColor(0.5, 0.5, 0.5, 1);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(size_t i = 0; i < primitiveList.size(); i++)
    {
        rendererGL->RenderWireframe(&camera, primitiveList[i]);
    }

    SwapBuffers(window.hdc);

}

void main()
{
    double frameTime = 16.0;

    FastTimer::Initialize();

    Init();

    LoadResources();

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

    Shutdown();
}