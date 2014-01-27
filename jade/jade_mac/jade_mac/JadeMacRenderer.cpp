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

jade::RenderDevice* device;
jade::TextureManager* texManager;
jade::Scene* scene;
jade::Renderer* rendererGL;
jade::Camera camera;
Window window;

void InitRenderer()
{
    std::string workingDir = WorkingDir();
    jade::RenderDeviceSetting deviceSetting;
	deviceSetting.msaaCount = 4;
    jade::InitRenderDevice(&window, &deviceSetting, &device);
    jade::InitRendererGL(device, &rendererGL);
	texManager = new jade::TextureManager(device);
	scene = new jade::Scene();
}

void ShutdownRenderer()
{
	delete texManager;
    jade::ShutdownRendererGL(rendererGL);
    jade::ShutdownRenderDevice(device);
}

void InitScene()
{
    std::vector<jade::Primitive* > primitiveList;
	Matrix4x4 flipMatrix = Matrix4x4(
                                     1.f, 0, 0, 0,
                                     0, 0, 1.f, 0,
                                     0, 1.f, 0, 0,
                                     0, 0, 0, 1.f);
    
	Matrix4x4 texflipMatrix =  Matrix4x4(
                                         1.f, 0.f, 0.f, 0.f,
                                         0.f, -1.f, 0.f, 1.f,
                                         0.f, 0.f, 1.f, 0.f,
                                         0.f, 0.f, 0.f, 1.f
                                         );
	jade::LoadFromObjMesh("data/sponza/sponza.obj", device, texManager,  flipMatrix, texflipMatrix, primitiveList);
	//jade::LoadFromObjMesh("data/db5/db5.obj", device, texManager, Translate(Vector3(0, 0, 15)) * Scale(Vector3(80, 80, 80)), texflipMatrix, primitiveList);
	scene->AddPrimitives(primitiveList);
    
	jade::Light* dirLight = new jade::DirectionLight();
	scene->AddLight(dirLight);
    
	jade::Light* pointLight = new jade::PointLight();
	scene->AddLight(pointLight);
    
}

void RenderFrame()
{
  	rendererGL->Render(&camera, scene);
}