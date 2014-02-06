#include <stdio.h>
#include "../window.h"
#include "../vector.h"
#include "../matrix.h"
#include "../Timer.h"
#include "../camera.h"
#include "../image.h"
#include "../obj_mesh.h"
#include "mesh.h"
#include "primitive.h"
#include "light.h"
#include "texture.h"
#include "scene.h"
#include "renderer_gl.h"
#include "camera.h"

Window window;
jade::RenderDevice* device;
jade::TextureManager* texManager;
jade::Scene* scene;
jade::Renderer* rendererGL;
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

void InputControl(float frameTime)
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
		camera.lookat.Normalize();
        camera.up = cross(camera.right, camera.lookat);
    }

    inputListener->ClearState();
}

void Init()
{
    if(!InitWindow(1024, 1024, window))
        return;

    jade::RenderDeviceSetting deviceSetting;
	deviceSetting.msaaCount = 4;
    jade::InitRenderDevice(&window, &deviceSetting, &device);
    jade::InitRendererGL(device, &rendererGL);    
	texManager = new jade::TextureManager(device);
	scene = new jade::Scene();
    inputListener = new MyInputListener();
    SetInputListener(inputListener);
}

void Shutdown()
{
	delete texManager;
    jade::ShutdownRendererGL(rendererGL);
    jade::ShutdownRenderDevice(device);
}

std::vector<jade::Primitive* > primitiveList;

void LoadResources()
{
	Matrix4x4 flipMatrix = Matrix4x4(		
		1.f, 0, 0, 0,
		0, 0, 1.f, 0,
		0, 1.f, 0, 0,
		0, 0, 0, 1.f);

	Matrix2x2 texflipMatrix =  Matrix2x2(
		1.f, 0.f,
		0.f, -1.f
		);

	ObjMesh objMesh;
	objMesh.Load("data/sponza/sponza.obj");

	jade::LoadFromObjMesh(objMesh, device, texManager,  flipMatrix, texflipMatrix, primitiveList);
	//jade::LoadFromObjMesh("data/db5/db5.obj", device, texManager, Translate(Vector3(0, 0, 15)) * Scale(Vector3(80, 80, 80)), texflipMatrix, primitiveList);
	scene->AddPrimitives(primitiveList);

	jade::Light* dirLight = new jade::DirectionLight(Vector3(1, -1, 1), Vector3(1, 1, 1) );
	scene->AddLight(dirLight);

	jade::Light* dirLight2 = new jade::DirectionLight(Vector3(0, -1, 1), Vector3(1, 1, 1) );
	//scene->AddLight(dirLight2);

	jade::Light* dirLight3 = new jade::DirectionLight(Vector3(1, 0, 1), Vector3(1, 1, 1) );
	//scene->AddLight(dirLight3);

	jade::Light* dirLight4 = new jade::DirectionLight(Vector3(-1, 0, 1), Vector3(1, 1, 1) );
	//scene->AddLight(dirLight4);

	//jade::Light* pointLight = new jade::PointLight();
	//scene->AddLight(pointLight);
	


}

void UnloadResources()
{
	primitiveList.clear();
	delete scene;
}

void Render(double frameTime)
{

	rendererGL->Render(&camera, scene);

    SwapBuffers(window.hdc);
}

void main()
{
    double frameTime = 16.0;

    FastTimer::Initialize();

    Init();

    LoadResources();

    FastTimer timer;


    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
		timer.Start();
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
            //if( GetMessage( &msg, NULL, 0, 0 ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        InputControl(frameTime);
        Render(frameTime);

		timer.End();
		frameTime = (float) timer.GetDurationInMillisecnds();
    }

	UnloadResources();
    Shutdown();
}