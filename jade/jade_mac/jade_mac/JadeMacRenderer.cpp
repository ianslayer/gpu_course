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
#include "../../../timer.h"

#include "rng.h"
#include "monte_carlo.h"
#include "brdf.h"

GLuint vao;

jade::RenderDevice* device;
jade::TextureManager* texManager;
jade::Scene* scene;
jade::Renderer* rendererGL;
jade::Renderer* rendererRT;
jade::Camera camera;
Window window;


MyInputListener* inputListener = NULL;


const static int numTestSamples = 16 * 16;
Vector3 samples3D[numTestSamples];
Vector2 samples2D[numTestSamples];
Vector2 sampleTriangle[numTestSamples];

void TestUniformSampleHemisphere()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples3D[i] = jade::UniformSampleHemisphere(rng.RandomFloat(), rng.RandomFloat());
	}
}

void TestSampleCosineHemiSphere()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples3D[i] = jade::CosineSampleHemisphere(rng.RandomFloat(), rng.RandomFloat());
	}
}

void TestUniformSampleSphere()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples3D[i] = jade::UniformSampleSphere(rng.RandomFloat(), rng.RandomFloat());
	}
}

void TestUniformSampleSquare()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples2D[i] = Vector2(rng.RandomFloat(), rng.RandomFloat());
	}
}

/*
 void TestSampleStratifiedSqure()
 {
 jade::RNG rng;
 StratifiedSample2D(samples2D, 16, 16, rng);
 }
 */

void TestSampleDisk()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples2D[i] = jade::UniformSampleDisk(rng.RandomFloat(), rng.RandomFloat());
	}
}

void TestSampleConcentricDisk()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples2D[i] = jade::ConcentricSampleDisk(rng.RandomFloat(), rng.RandomFloat());
	}
}

void TestSampleTriangle()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		Vector2 sample =jade::UniformSampleTriangle(rng.RandomFloat(), rng.RandomFloat());
		samples3D[i] = Vector3(sample.x, sample.y, 0);
	}
}

void TestSampleBlinnNDF(Vector3 wo)
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		float pdf = 0.f;
		while(pdf == 0.f)
			jade::SampleBlinnNDF(wo, samples3D[i], 20, rng.RandomFloat(), rng.RandomFloat(), pdf);
		
	}
}

void TestSamplePrim(jade::Primitive* prim)
{
	jade::RNG rng;
	jade::PrimitiveSampler sampler(prim);
	
	for(int i = 0; i < numTestSamples; i++)
	{
		Vector3 normal;
		samples3D[i] = sampler.Sample(rng.RandomFloat(), rng.RandomFloat(), rng.RandomFloat(), normal);
	}
}

void Draw3DSamples(jade::Renderer* renderer, const jade::Camera* camera, const Vector3& origin, const Vector3* samples, int numSamples)
{
	for(int i = 0; i < numSamples; i++)
	{
		jade::AABB bound;
		bound.center = origin + samples[i];
		bound.radius = Vector3(0.01f);
		
		renderer->DrawBoundingBox(camera, bound);
		
	}
}

void Draw2DSamples(jade::Renderer* renderer, const jade::Camera* camera, const Vector3& origin, const Vector2* samples, int numSamples)
{
	for(int i = 0; i < numSamples; i++)
	{
		jade::AABB bound;
		bound.center = origin + Vector3(samples[i].x, samples[i].y, 0);
		bound.radius = Vector3(0.01f);
		
		renderer->DrawBoundingBox(camera, bound);
		
	}
}


void InputControl(float frameTime)
{
    FastTimer timer;
	timer.Start();
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
    timer.End();
    inputListener->ClearState();
}

void InitRenderer(float width, float height, float scaleFactor)
{
    std::string workingDir = WorkingDir();
    jade::RenderDeviceSetting deviceSetting;
	deviceSetting.msaaCount = 4;
    deviceSetting.screenScaleFactor = scaleFactor;
    
    window.width = width;
    window.height = height;
    
    jade::InitRenderDevice(&window, &deviceSetting, &device);
    jade::InitRendererGL(device, &rendererGL);
    
    jade::InitRendererRT(device, &rendererRT);
    
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
	jade::ShutdownRendererRT(rendererRT);
    jade::ShutdownRenderDevice(device);
}

void SetCastShadow(std::vector<jade::Primitive* >& primitiveList)
{
	for(size_t i = 0; i < primitiveList.size(); i++)
	{
		primitiveList[i]->castShadow = true;
	}
}

void InitScene()
{
	//TestSampleBlinnNDF(Normalize(Vector3(0, 0, 1) ));
//	TestUniformSampleHemisphere();

//	TestSampleTriangle();
	
	Matrix4x4 flipMatrix = Matrix4x4(
                                     1.f, 0, 0, 0,
                                     0, 0, 1.f, 0,
                                     0, 1.f, 0, 0,
                                     0, 0, 0, 1.f);
    
	Matrix2x2 texflipMatrix =  Matrix2x2(
                                         1.f, 0.f,
                                         0.f, -1.f
                                         );
    
	std::vector<jade::Primitive* > primitiveList, primitiveList2, primitiveList3;
	ObjMesh objMesh, objMesh2, objMesh3;
	objMesh.Load("data/sponza/sponza.obj");
	objMesh2.Load("data/db5/db5.obj");
	objMesh3.Load("data/sphere.obj");
	//jade::LoadFromObjMesh(objMesh, device, texManager,  flipMatrix, texflipMatrix, primitiveList);
	//jade::LoadFromObjMesh(objMesh2, device, texManager, Translate(Vector3(0, 0, 15)) * Scale(Vector3(80, 80, 80)), texflipMatrix, primitiveList2);
	jade::LoadFromObjMesh(objMesh3, device, texManager, Translate(Vector3(-200, 0, 40)) * Scale(Vector3(40, 40, 40)), texflipMatrix, primitiveList3);
    //SetCastShadow(primitiveList);
	SetCastShadow(primitiveList2);
    
	//scene->AddPrimitives(primitiveList);
	scene->AddPrimitives(primitiveList2);
	scene->AddPrimitives(primitiveList3);
	
	TestSamplePrim(primitiveList3[0]);
	
	jade::Light* dirLight = new jade::DirectionLight(Normalize(Vector3(1, -1, 1)), Vector3(1, 1, 1) );
	scene->AddLight(dirLight);
    
	jade::Light* pointLight = new jade::PointLight(Vector3(200, 0, 200), 500 * Vector3(900, 600, 600), 10 );
	//scene->AddLight(pointLight);
	
	jade::Light* pointLight2 = new jade::PointLight(Vector3(-200, 0, 100), 500 * Vector3(0.5, 0.5, 0.8), 20 );
	//scene->AddLight(pointLight2);
	
	jade::Mesh* cubeMesh = CreateMeshCube(device);
	jade::Primitive* cubePrim = CreateCube(texManager, Vector3(0.f, 0.f, -200.f), Vector3(200.f), cubeMesh);

	scene->AddPrimitive(cubePrim);
}

void RayTraceScreenShot()
{
	rendererRT->ScreenShot("rt.tga", &camera, scene);
}

void RenderFrame()
{
  	rendererGL->Render(&camera, scene);
	Vector3 offset = Vector3(0.f);
	Draw3DSamples(rendererGL, &camera, offset, samples3D, numTestSamples);
}