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
#include "renderer_hw1.h"
#include "camera.h"

Window window;
jade::RenderDevice* device;
jade::TextureManager* texManager;
jade::Scene* scene;
jade::Renderer* renderer;
jade::Camera camera;

float cameraMoveSpeed = 100.f;
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

		if(key == KEY_R && pressed == false)
		{
			options.reloadShaders = true;
		}

		if(key == KEY_0 && pressed == false)
		{
			options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_NONE;
		}

		if(key == KEY_1 && pressed == false )
		{
			options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_UV_TILING;
		}

		if(key == KEY_2 && pressed == false)
		{
			options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_TANGENT_SPACE;
		}

		if(key == KEY_3 && pressed == false)
		{
			options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_DIFFUSE;
		}
		if(key == KEY_4 && pressed == false)
		{
			options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_NORMAL;
		}
		if(key == KEY_5 && pressed == false)
		{
			options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_SPECULAR;
		}
		if(key == KEY_6 && pressed == false)
		{
			options.dbgDraw =jade::GLRendererOptions::DBG_DRAW_DIFFUSE_LIGHTING;
		}

		if(key == KEY_7 && pressed == false)
		{
			options.dbgDraw =jade::GLRendererOptions::DBG_DRAW_SPECULAR_LIGHTING;
		}

		if(key == KEY_8 && pressed == false)
		{
			options.dbgDraw =jade::GLRendererOptions::DBG_DRAW_FRESNEL_SPECULAR_LIGHTING;
		}

		if(key == KEY_9 && pressed == false)
		{
			options.dbgDraw =jade::GLRendererOptions::DBG_DRAW_SHADOW_MAP;
		}
    }

    void ClearState()
    {
        rx = 0;
        ry = 0;

		options.reloadShaders = false;
    }

    bool forward;
    bool backward;
    bool right;
    bool left;

    bool mouseControl;
    float rx;
    float ry;

	jade::GLRendererOptions options;
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
	renderer->SetRendererOption(&inputListener->options);

    inputListener->ClearState();
}

void Init()
{
    if(!InitWindow(1024, 768, window))
        return;
	camera.SetAspectRatio(1024, 768);
    jade::RenderDeviceSetting deviceSetting;
	deviceSetting.msaaCount = 4;
    jade::InitRenderDevice(&window, &deviceSetting, &device);
    jade::InitRendererGL(device, &renderer);    
	//jade::InitRendererHW(device, &renderer);
	texManager = new jade::TextureManager(device);
	scene = new jade::Scene();
    inputListener = new MyInputListener();
    SetInputListener(inputListener);
}

void Shutdown()
{
	delete texManager;
    
	jade::ShutdownRendererGL(renderer);
//  jade::ShutdownRendererHW(renderer);
	jade::ShutdownRenderDevice(device);
}

void SetCastShadow(std::vector<jade::Primitive* >& primitiveList)
{
	for(size_t i = 0; i < primitiveList.size(); i++)
	{
		primitiveList[i]->castShadow = true;
	}
}

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

	std::vector<jade::Primitive* > primitiveList, primitiveList2;
	ObjMesh objMesh, objMesh2;
	objMesh.Load("data/sponza/sponza.obj");
	objMesh2.Load("data/db5/db5.obj");
	jade::LoadFromObjMesh(objMesh, device, texManager,  flipMatrix, texflipMatrix, primitiveList);
	jade::LoadFromObjMesh(objMesh2, device, texManager, Translate(Vector3(0, 0, 15)) * Scale(Vector3(80, 80, 80)), texflipMatrix, primitiveList2);

	SetCastShadow(primitiveList2);

	scene->AddPrimitives(primitiveList);
	scene->AddPrimitives(primitiveList2);
	jade::Light* dirLight = new jade::DirectionLight(Normalize(Vector3(1, -1, 1)), Vector3(0.6, 0.6, 0.6) );
	scene->AddLight(dirLight);

	jade::Light* pointLight = new jade::PointLight(Vector3(1000, 0, 50), Vector3(0.9, 0.6, 0.6), 100 );
	scene->AddLight(pointLight);
	
	jade::Light* pointLight2 = new jade::PointLight(Vector3(-1000, 0, 10), Vector3(0.5, 0.5, 0.8), 200 );
	scene->AddLight(pointLight2);
}

void UnloadResources()
{
	delete scene;
}

void Render(double frameTime)
{

	renderer->Render(&camera, scene);

    SwapBuffers(window.hdc);
}

void PrintUsage()
{
	printf("key control:\n");
	printf("w, a, s, d, mouse moving\n");
	printf("key 0: normal rendering\n");
	printf("key 1: show texture UV\n");
	printf("key 2: show tangent space\n");
	printf("key 3: draw diffuse\n");
	printf("key 4: draw normal\n");
	printf("key 5: draw roughness\n");
	printf("key 6: draw diffuse lighting\n");
	printf("key 7: draw specular lighting without Fresnel\n");
	printf("key 8: draw specular lighting with Fresnel\n");
	printf("key 9: show shadow map\n");
	printf("key r: re compile shader\n");
	printf("\n");
}

void main()
{
    double frameTime = 16.0;

    FastTimer::Initialize();

	PrintUsage();

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