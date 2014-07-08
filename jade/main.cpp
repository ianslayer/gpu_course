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
#include "renderer_ray_tracing.h"
#include "camera.h"
#include "rng.h"
#include "monte_carlo.h"
#include "brdf.h"

Window window;
jade::RenderDevice* device;
jade::TextureManager* texManager;
jade::Scene* scene;
jade::Renderer* renderer;
jade::Renderer* rendererRT;
jade::Camera camera;

float cameraMoveSpeed = 1.f;
float cameraTurnSpeed = 0.5f;


enum DrawSampleMode
{
	SAMPLE_UNIFORM,
	SAMPLE_LIGHT,
	SAMPLE_BRDF,
};

DrawSampleMode drawSampleMode = SAMPLE_UNIFORM;

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

	virtual void OnMouseButton(const int x, const int y, const MouseButton button, const bool pressed);

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
			//options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_NONE;
			drawSampleMode = SAMPLE_UNIFORM;
		}

		if(key == KEY_1 && pressed == false )
		{
			//options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_UV_TILING;
			drawSampleMode = SAMPLE_LIGHT;
		}

		if(key == KEY_2 && pressed == false)
		{
			//options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_TANGENT_SPACE;
			drawSampleMode = SAMPLE_BRDF;
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

		if(key == KEY_V && pressed == false)
		{
			options.shadowTech = jade::GLRendererOptions::SHADOW_VARIANCE_SHADOW_MAP;
		}

		if(key == KEY_P && pressed == false)
		{
			options.shadowTech = jade::GLRendererOptions::SHADOW_MAP_PCF;
		}

		if(key == KEY_M && pressed == false)
		{
			options.screenShot = true;
			rendererRT->ScreenShot("rt.tga", &camera, scene);
		}

		if(key == KEY_C && pressed == false)
		{
			jade::SaveCameraParameter("cam.cam", camera);
		}


    }

    void ClearState()
    {
        rx = 0;
        ry = 0;

		options.reloadShaders = false;
		options.screenShot = false;
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

	LoadCameraParameter("cam.cam", camera);

    jade::RenderDeviceSetting deviceSetting;
	deviceSetting.msaaCount = 4;
    jade::InitRenderDevice(&window, &deviceSetting, &device);
    jade::InitRendererGL(device, &renderer);    
	jade::InitRendererRT(device, &rendererRT);
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
	jade::ShutdownRendererRT(rendererRT);
//  jade::ShutdownRendererHW(renderer);
	jade::ShutdownRenderDevice(device);
}


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
			jade::SampleBlinnNDF(wo, samples3D[i], 50, rng.RandomFloat(), rng.RandomFloat(), pdf);

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
const static int nSamples = 1024;
Vector3 uniformSample[nSamples]; Vector3* hitUniformSample[nSamples]; Vector3* misUniformSample[nSamples]; int numHitUniformSamples = 0; int numMisUniformSamples = 0;
Vector3 lightSample[nSamples]; Vector3* hitLightSample[nSamples]; Vector3* misLightSample[nSamples]; int numHitLightSample = 0; int numMisLightSample = 0;
Vector3 brdfSample[nSamples]; Vector3* hitBrdfSample[nSamples]; Vector3* misBrdfSample[nSamples]; int numHitBrdfSample = 0; int numMisBrdfSample = 0;

bool TestSampleDirectLighting(const jade::Scene* scene, const jade::SceneAccelerator* accelerator, const jade::Ray& viewRay, jade::Intersection& isect)
{
	jade::RNG rng;
	//get intersection point

	if(!Intersect(viewRay, accelerator, isect))
		return false;

	//uniform sample
	numHitUniformSamples = numMisUniformSamples = 0;
	Vector3 Ld(0.f);
	for(int i = 0; i < nSamples; i++)
	{

		Vector3 tangentDir = jade::UniformSampleHemisphere(rng.RandomFloat(), rng.RandomFloat());
		Vector3 wi = jade::TransformToWorldSpace(isect.n, isect.tangent, tangentDir);
		uniformSample[i] = wi;

		jade::Intersection lightIsect;
		jade::Ray sampleRay(isect.p + 0.0001f * wi, wi);
		if(Intersect(sampleRay, accelerator, lightIsect) )
		{
			
			uniformSample[i] = (lightIsect.p  - isect.p);

			hitUniformSample[numHitUniformSamples] = &uniformSample[i];

			numHitUniformSamples++;
	
		}
		else
		{
			misUniformSample[numMisUniformSamples] = &uniformSample[i];
			numMisUniformSamples++;
		}
	}

	return true;
}

bool TestSampleImportanceSampleLight(const jade::Scene* scene, const jade::SceneAccelerator* accelerator, const jade::Ray& viewRay, int lightIdx, jade::Intersection& isect)
{
	jade::RNG rng;
	//get intersection point
	numHitLightSample = numMisLightSample = 0;

	if(lightIdx > scene->lightList.size())
		return false;

	if(!Intersect(viewRay, accelerator, isect))
		return false;	

	Vector3 Ld = Vector3(0.f);

	Vector3 wo = -viewRay.direction;
	
	for(int j = 0; j < nSamples; j++)
	{

		//sample light
		Vector3 wi;


		float lightPdf, brdfPdf;
		jade::Range lightRange;

		Vector3 Li = Sample(*scene->lightList[lightIdx], isect.p, wi, lightRange, lightPdf);

		lightSample[j] = wi;

		Vector3 tangentWo = jade::TransformToTangentSpace(isect.n, isect.tangent, wo);
		Vector3 tangentWi = jade::TransformToTangentSpace(isect.n, isect.tangent, wi);


		if(lightPdf > 0.f)
		{

			jade::Ray shadowRay(isect.p, wi);
			float vis = SampleVisibility(shadowRay, lightRange, 0.0001f, accelerator);
			
			if(vis == 1.f)
			{
				lightSample[j] = wi * lightRange.tmax;
				hitLightSample[numHitLightSample++] = &lightSample[j];
			}

			else
			{
				misLightSample[numMisLightSample] = &lightSample[j];
				numMisLightSample++;
			}
		}
		
	}

	return false;

}

bool TestSampleImportanceSampleBRDF(const jade::Scene* scene, const jade::SceneAccelerator* accelerator, const jade::Ray& viewRay, int lightIdx, jade::Intersection& isect)
{
	jade::RNG rng;

	if(lightIdx > scene->lightList.size())
		return false;

	if(!Intersect(viewRay, accelerator, isect))
		return false;	

	numHitBrdfSample = numMisBrdfSample = 0;

	Vector3 wo = -viewRay.direction;
	Vector3 tangentWo = jade::TransformToTangentSpace(isect.n, isect.tangent, wo);

	float brdfPdf;

	for(int j = 0; j < nSamples; j++)
	{
		Vector3 tangentWi;
		jade::SampleBlinnNDF(tangentWo, tangentWi, 1024.f, rng.RandomFloat(), rng.RandomFloat(), brdfPdf);


		Vector3 wi = jade::TransformToWorldSpace(isect.n, isect.tangent, tangentWi);

		brdfSample[j] = wi;

		float weight = 1.f;

		if(brdfPdf > 0.f)
		{

			jade::Ray ray(isect.p + 0.00001 * wi, wi);
			jade::Intersection lightIsect;

			if(Intersect(ray, accelerator, lightIsect) )
			{
				if(lightIsect.prim->areaLight == scene->lightList[lightIdx])
				{
					brdfSample[j] = lightIsect.p - isect.p;
					hitBrdfSample[numHitBrdfSample++] = &brdfSample[j];					
				}
				else
				{
					misBrdfSample[numMisBrdfSample++] = &brdfSample[j];
				}
			}
			else
			{
				misBrdfSample[numMisBrdfSample++] = &brdfSample[j];
			}
				
			
		}
	}
}

jade::Intersection rayIsect;
jade::Ray viewRay;


void DrawLightSample(jade::Renderer* renderer, const jade::Camera* camera, const jade::Ray& viewRay, jade::Intersection& isect)
{
	renderer->DrawLine(camera, viewRay.origin, isect.p, Vector3(1, 0, 0));

	if(drawSampleMode == SAMPLE_UNIFORM)
	{
		for(int i = 0; i < nSamples; i++)
		{
			renderer->DrawLine(camera, isect.p, isect.p + Normalize(uniformSample[i]), Vector3(0, 1, 0));
		}

		for(int i = 0; i < numHitUniformSamples; i++)
		{
			renderer->DrawLine(camera, isect.p, isect.p + *hitUniformSample[i], Vector3(0, 0, 1));

			jade::AABB bound;
			bound.center = isect.p + *hitUniformSample[i];
			bound.radius = Vector3(samplePointSize);

			renderer->DrawBoundingBox(camera, bound);
		}
	}

	if(drawSampleMode == SAMPLE_LIGHT)
	{
		for(int i = 0; i < nSamples; i++)
		{
			renderer->DrawLine(camera, isect.p, isect.p + Normalize(lightSample[i]), Vector3(0, 1, 0));
		}
		
		for(int i = 0; i < numHitLightSample; i++)
		{
			renderer->DrawLine(camera, isect.p, isect.p + *hitLightSample[i], Vector3(0, 0, 1));

			jade::AABB bound;
			bound.center = isect.p + *hitLightSample[i];
			bound.radius = Vector3(samplePointSize);

			renderer->DrawBoundingBox(camera, bound);
		}
	}

	if(drawSampleMode == SAMPLE_BRDF)
	{
		for(int i = 0; i < nSamples; i++)
		{
			renderer->DrawLine(camera, isect.p, isect.p + Normalize(brdfSample[i]), Vector3(0, 1, 0));
		}

		
		for(int i = 0; i < numHitBrdfSample; i++)
		{
			renderer->DrawLine(camera, isect.p, isect.p + *hitBrdfSample[i], Vector3(0, 0, 1));

			jade::AABB bound;
			bound.center = isect.p + *hitBrdfSample[i];
			bound.radius = Vector3(samplePointSize);

			renderer->DrawBoundingBox(camera, bound);
		}
	}
}


void MyInputListener::OnMouseButton(const int x, const int y, const MouseButton button, const bool pressed)
{
	if(pressed && button == MOUSE_RIGHT)
	{
		mouseControl = true;
	}
	else
	{
		mouseControl = false;
	}

	if(pressed == false && button == MOUSE_LEFT)
	{
		viewRay = jade::Ray(camera.position, camera.lookat);
		TestSampleDirectLighting(scene, rendererRT->GetAccelerator(), viewRay,rayIsect);
		TestSampleImportanceSampleLight(scene, rendererRT->GetAccelerator(), viewRay, 1, rayIsect);
		TestSampleImportanceSampleBRDF(scene, rendererRT->GetAccelerator(), viewRay, 1, rayIsect);
	}
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

void TestSample()
{
	TestSampleCosineHemiSphere();
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
	//TestSample();
//	TestSampleBlinnNDF(Normalize(Vector3(0, 0, 1) ));
//	TestSampleWrongDisk();
	TestSampleDisk();



	Matrix4x4 flipMatrix = Matrix4x4(		
		1.f, 0, 0, 0,
		0, 0, -1.f, 0,
		0, 1.f, 0, 0,
		0, 0, 0, 1.f);

	Matrix2x2 texflipMatrix =  Matrix2x2(
		1.f, 0.f,
		0.f, -1.f
		);

	std::vector<jade::Primitive* > primitiveList, primitiveList2, primitiveList3;
	ObjMesh objMesh, objMesh2, objMesh3;
	//objMesh.Load("data/sponza/sponza.obj");
	//objMesh2.Load("data/db5/db5.obj");
	//objMesh3.Load("data/sphere.obj");
	//jade::LoadFromObjMesh(objMesh, device, texManager,  flipMatrix, texflipMatrix, primitiveList);
	//jade::LoadFromObjMesh(objMesh2, device, texManager, Translate(Vector3(0, 0, 15)) * Scale(Vector3(80, 80, 80)), texflipMatrix, primitiveList2);
	//jade::LoadFromObjMesh(objMesh3, device, texManager, Translate(Vector3(-200, 0, 400)) * Scale(Vector3(40, 40, 40)), texflipMatrix, primitiveList3);

	//SetCastShadow(primitiveList);
	SetCastShadow(primitiveList2);
	jade::Mesh* cubeMesh = CreateMeshCube(device);
	jade::Primitive* cubePrim = CreateCube(texManager, Vector3(0.f, 0.f, -200.f), Vector3(200.f), cubeMesh);

	//TestSamplePrim(primitiveList3[0]);

	//cubePrim->castShadow = true;
	scene->AddPrimitive(cubePrim);

	//scene->AddPrimitives(primitiveList);
	//scene->AddPrimitives(primitiveList2);
	//scene->AddPrimitives(primitiveList3);


	/*
	jade::Light* dirLight = new jade::DirectionLight(Normalize(Vector3(1, -1, 1)), Vector3(0.6, 0.6, 0.6) );
	scene->AddLight(dirLight);	

	jade::Light* pointLight = new jade::PointLight(Vector3(500, 0, 100), Vector3(0.9, 0.6, 0.6), 100 );
	scene->AddLight(pointLight);
	
	jade::Light* pointLight2 = new jade::PointLight(Vector3(-1000, 0, 10), Vector3(0.5, 0.5, 0.8), 200 );
	scene->AddLight(pointLight2);
	*/

	jade::Light* dirLight = new jade::DirectionLight(Normalize(Vector3(1, 0.3, 0.5)), Vector3(0.6, 0.6, 0.6) );
	scene->AddLight(dirLight);

	jade::Light* pointLight = new jade::PointLight(Vector3(200, 0, 200), 500 * Vector3(0.9, 0.6, 0.6), 10 );
	//scene->AddLight(pointLight);

	jade::Light* pointLight2 = new jade::PointLight(Vector3(-200, 0, 100), 500 * Vector3(0.5, 0.5, 0.8), 20 );
	//scene->AddLight(pointLight2);

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

	rendererRT->GetAccelerator()->Build(scene);
	viewRay = jade::Ray(camera.position, camera.lookat);
	TestSampleDirectLighting(scene, rendererRT->GetAccelerator(), viewRay,rayIsect);
	TestSampleImportanceSampleLight(scene, rendererRT->GetAccelerator(), viewRay, 1, rayIsect);
	TestSampleImportanceSampleBRDF(scene, rendererRT->GetAccelerator(), viewRay, 1, rayIsect);
}

void UnloadResources()
{
	delete scene;
}

void Render(double frameTime)
{
	
	renderer->Render(&camera, scene);

	Vector3 offset = Vector3(0.f);
	//Draw3DSamples(renderer, &camera, offset, samples3D, numTestSamples);
	DrawLightSample(renderer, &camera, viewRay, rayIsect);
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