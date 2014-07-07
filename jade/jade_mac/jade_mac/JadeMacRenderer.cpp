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
bool drawSampleLine = false;
float samplePointSize = 1.f;

void TestUniformSampleHemisphere()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples3D[i] = jade::UniformSampleHemisphere(rng.RandomFloat(), rng.RandomFloat());
	}
	drawSampleLine= true;
	samplePointSize = 0.01;
}

void TestSampleCosineHemiSphere()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples3D[i] = jade::CosineSampleHemisphere(rng.RandomFloat(), rng.RandomFloat());
	}
	drawSampleLine= true;
	samplePointSize = 0.01;
}

void TestUniformSampleSphere()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples3D[i] = jade::UniformSampleSphere(rng.RandomFloat(), rng.RandomFloat());
	}
	drawSampleLine= false;
	samplePointSize = 0.01;
}

void TestUniformSampleSquare()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples3D[i] = Vector3(rng.RandomFloat(), rng.RandomFloat(), 0.f);
	}
	drawSampleLine= false;
	samplePointSize = 0.01;
}

/*
 void TestSampleStratifiedSqure()
 {
 jade::RNG rng;
 StratifiedSample2D(samples2D, 16, 16, rng);
 }
 */

void TestSampleWrongDisk()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		float r = rng.RandomFloat();
		float theta = 2 * M_PI * rng.RandomFloat();
		samples3D[i] = Vector3( r * cos(theta), r* sin(theta), 0);
	}
	drawSampleLine= false;
	samplePointSize = 0.01;
}

void TestSampleDisk()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		Vector2 sample = jade::UniformSampleDisk(rng.RandomFloat(), rng.RandomFloat());
		samples3D[i] = Vector3(sample.x, sample.y, 0);
	}
	drawSampleLine= false;
	samplePointSize = 0.01;
}

void TestSampleConcentricDisk()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples2D[i] = jade::ConcentricSampleDisk(rng.RandomFloat(), rng.RandomFloat());
	}
	drawSampleLine= false;
	samplePointSize = 0.01;
}

void TestSampleTriangle()
{
	jade::RNG rng;
	for(int i = 0; i < numTestSamples; i++)
	{
		samples2D[i] = jade::UniformSampleTriangle(rng.RandomFloat(), rng.RandomFloat());
	}
	drawSampleLine= false;
	samplePointSize = 0.01;
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
	drawSampleLine= true;
	samplePointSize = 0.01;
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
	samplePointSize = 1.f;
}

const static int MAX_LIGHT = 4;
Vector3 lightSample[MAX_LIGHT][numTestSamples];
Vector3 brdfSample[MAX_LIGHT][numTestSamples];

void SampleDirectLighting(const jade::Scene* scene, const jade::Ray& viewRay)
{
	
}

void DrawCoordinate(const jade::Camera* camera, jade::Renderer* renderer)
{
	Vector3 origin(0, 0, 0);
	
	Vector3 red(1, 0, 0);
	Vector3 x(1, 0, 0);
	Vector3 green(0, 1, 0);
	Vector3 y(0, 1, 0);
	Vector3 blue(0, 0, 1);
	Vector3 z(0, 0, 1);
	renderer->DrawLine(camera, origin, x, red);
	renderer->DrawLine(camera, origin, y, green);
	renderer->DrawLine(camera, origin, z, blue);
}

void Draw3DSamples(jade::Renderer* renderer, const jade::Camera* camera, const Vector3& origin, const Vector3* samples, int numSamples)
{
	DrawCoordinate(camera, renderer);
	for(int i = 0; i < numSamples; i++)
	{
		jade::AABB bound;
		bound.center = origin + samples[i];
		bound.radius = Vector3(samplePointSize);
		
		renderer->DrawBoundingBox(camera, bound);
		Vector3 red(0, 0, 1);
		if(drawSampleLine)
			renderer->DrawLine(camera, origin, samples[i], red);
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
	TestSampleBlinnNDF(Normalize(Vector3(0, 0, 1) ));
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
	jade::LoadFromObjMesh(objMesh2, device, texManager, Translate(Vector3(0, 0, 15)) * Scale(Vector3(80, 80, 80)), texflipMatrix, primitiveList2);
	//jade::LoadFromObjMesh(objMesh3, device, texManager, Translate(Vector3(-200, 0, 40)) * Scale(Vector3(40, 40, 40)), texflipMatrix, primitiveList3);
	
    //SetCastShadow(primitiveList);
	SetCastShadow(primitiveList2);
    
	//scene->AddPrimitives(primitiveList);
	//scene->AddPrimitives(primitiveList2);
	//scene->AddPrimitives(primitiveList3);
	

	
	jade::Light* dirLight = new jade::DirectionLight(Normalize(Vector3(1, -1, 1)), Vector3(500, 500, 500) );
	//scene->AddLight(dirLight);
    
	jade::Light* pointLight = new jade::PointLight(Vector3(200, 0, 200), 500 * Vector3(900, 600, 600), 10 );
	//scene->AddLight(pointLight);
	
	jade::Light* pointLight2 = new jade::PointLight(Vector3(-200, 0, 100), 500 * Vector3(500, 500, 800), 20 );
	//scene->AddLight(pointLight2);
	

	
	jade::Mesh* cubeMesh = CreateMeshCube(device);
	jade::Primitive* cubePrim = CreateCube(texManager, Vector3(0.f, 0.f, -200.f), Vector3(200.f), cubeMesh);
	jade::Primitive* cubePrimLight = CreateCube(texManager, Vector3(-50, 0, 40), Vector3(40.f), cubeMesh);
	jade::Primitive* cubePrimLight2 = CreateCube(texManager, Vector3(20, 0, 10), Vector3(10.f), cubeMesh);
	//TestSamplePrim(cubePrimLight);

	
	jade::GeomAreaLight* areaLight = new jade::GeomAreaLight(Vector3(0.8, 0.2, 0.2), cubePrimLight);
	jade::GeomAreaLight* areaLight2 = new jade::GeomAreaLight(Vector3(0.2, 0.5, 0.2), cubePrimLight2);
	
	jade::RNG rng;
	
	for(int i = 0; i < numTestSamples; i++)
	{
		Vector3 normal;
		samples3D[i] = areaLight->sampler.Sample(rng.RandomFloat(), rng.RandomFloat(), rng.RandomFloat(), normal);
		samplePointSize = 1.f;
	}
	
	scene->AddLight(areaLight);
	scene->AddLight(areaLight2);
	
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