//
//  JadeMacRenderer.cpp
//  jade_mac
//
//  Created by Yushuo Liou on 1/27/14.
//  Copyright (c) 2014 ianslayer. All rights reserved.
//

#include "JadeMacRenderer.h"
#include "../../jade.h"
#include "../../../window.h"
#include "../../../file_utility.h"
#include "../../../obj_mesh.h"
#include "../../../input.h"

GLuint vao;

jade::RenderDevice* device;
jade::TextureManager* texManager;
jade::Scene* scene;
jade::Renderer* rendererGL;
jade::Camera camera;
Window window;


MyInputListener* inputListener = NULL;

void InputControl(float frameTime)
{
    
    if(inputListener->forward)
    {
        camera.position += camera.lookat * inputListener->cameraMoveSpeed * frameTime / 1000.f;
    }
    if(inputListener->backward)
    {
        camera.position -= camera.lookat * inputListener->cameraMoveSpeed * frameTime / 1000.f;
    }
    if(inputListener->right)
    {
        camera.position += camera.right * inputListener->cameraMoveSpeed * frameTime / 1000.f;
    }
    if(inputListener->left)
    {
        camera.position -= camera.right * inputListener->cameraMoveSpeed * frameTime / 1000.f;
    }
    
    if(inputListener->mouseControl)
    {
        Matrix3x3 head = Matrix3x3::RotateAxis(camera.up, -inputListener->rx * inputListener->cameraTurnSpeed * frameTime / 1000.f);
        camera.lookat = head * camera.lookat;
        camera.right = head *  camera.right;
        
        Matrix3x3 pitch = Matrix3x3::RotateAxis(camera.right, -inputListener->ry * inputListener->cameraTurnSpeed * frameTime / 1000.f);
        camera.lookat = pitch * camera.lookat;
        camera.up = pitch *  camera.up;
        
        //make sure right is horizontal
        camera.right.z = 0;
        camera.right.Normalize();
        camera.lookat = cross(camera.up, camera.right);
		camera.lookat.Normalize();
        camera.up = cross(camera.right, camera.lookat);
    }
    
    rendererGL->SetRendererOption(&inputListener->options);
    
    inputListener->ClearState();
}

void InitRenderer(float width, float height)
{
    std::string workingDir = WorkingDir();
    jade::RenderDeviceSetting deviceSetting;
	deviceSetting.msaaCount = 4;
    
    jade::InitRenderDevice(&window, &deviceSetting, &device);
    jade::InitRendererGL(device, &rendererGL);
    
    //jade::InitRendererHW(device, &rendererGL);
    
	texManager = new jade::TextureManager(device);
	scene = new jade::Scene();
  
    camera.SetAspectRatio(width, height);
    
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
    
   // InitResources();
    
    InitInput();
}

void ShutdownRenderer()
{
	delete texManager;
    jade::ShutdownRendererGL(rendererGL);
    jade::ShutdownRenderDevice(device);
}

void InitScene()
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
    
	std::vector<jade::Primitive* > primitiveList;
	ObjMesh objMesh, objMesh2;
	objMesh.Load("data/sponza/sponza.obj");
	objMesh2.Load("data/db5/db5.obj");
	jade::LoadFromObjMesh(objMesh, device, texManager,  flipMatrix, texflipMatrix, primitiveList);
	jade::LoadFromObjMesh(objMesh2, device, texManager, Translate(Vector3(0, 0, 15)) * Scale(Vector3(80, 80, 80)), texflipMatrix, primitiveList);
	scene->AddPrimitives(primitiveList);
    
	jade::Light* dirLight = new jade::DirectionLight(Vector3(1, -1, 1), Vector3(0.6, 0.6, 0.6) );
	scene->AddLight(dirLight);
    
	jade::Light* pointLight = new jade::PointLight(Vector3(1000, 0, 50), Vector3(0.9, 0.6, 0.6), 100 );
	scene->AddLight(pointLight);
	
	jade::Light* pointLight2 = new jade::PointLight(Vector3(-1000, 0, 10), Vector3(0.5, 0.5, 0.8), 200 );
	scene->AddLight(pointLight2);
}

void RenderFrame()
{
  	rendererGL->Render(&camera, scene);
}