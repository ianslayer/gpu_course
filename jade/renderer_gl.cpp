#include "renderer_gl.h"
#include "../window.h"
#include "render_device.h"
#include "camera.h"
#include "primitive.h"
#include "material.h"
#include "texture.h"
#include "mesh.h"
#include "scene.h"
#include "light.h"
#include "../matrix.h"
#include "../image.h"

namespace jade
{
	class RendererGL : public Renderer
	{
	public:
		RendererGL(RenderDevice* _device);
		virtual ~RendererGL();

		virtual void Render(const Camera* camera, const Scene* scene);
		virtual void ScreenShot(const char* path, const Camera* camera, const Scene* scene);
		
		void RenderGBuffer(const Camera* camera, const Scene* scene);
		//void RenderShadowMap(const Camera* camera, const Scene* scene);
		void RenderShadowMap(const PointLight* light, const Scene* scene, const Camera* cam);
		void RenderShadowMap(const DirectionLight* light, const AABB& bound,  const Scene* scene, const Camera* cam);
		void ClearDeferredShadow();
		void AccumDeferredShadow(const Matrix4x4& shadowMapMatrix, float minDepth, float maxDepth, const Matrix4x4& shadowViewMatrix, const Camera* cam);
        
		virtual void SetRendererOption(void*);

		void ReloadShaders();
        
		void RenderDebugInfo(const Camera* camera, const Scene* scene);

		void DrawBoundingBox(const Camera* camera, const AABB& bound);
		void DrawLightBounding(const Camera* camera, const Scene* scene);
		
		void DrawTexture(int x, int y, int width, int height, const HWTexture2D* tex);
        void GaussianBlur(HWRenderTexture2D* src, HWRenderTexture2D* temp);

		class RenderDevice* device;
		GLuint wireframeShader;
		GLuint matShader;
		GLuint blitShader;
		GLuint shadowCasterShader;
		GLuint shadowCasterPointLightShader;
		GLuint gbufferShader;
		GLuint deferredShadowShader;
		GLuint gaussianBlurShader;
        
		GLuint whiteTexture;
		GLuint blackTexture;
		GLuint uniZTexture; //unit z
        GLuint noiseTexture;

		GLuint cubeVbo;
		GLuint cubeIbo;
        
		RefCountedPtr<TextureSamplerState> defaultSamplerState;
		RefCountedPtr<TextureSamplerState> mipMapClampSamplerState;
		RefCountedPtr<TextureSamplerState> pointSamplerState;
		RefCountedPtr<TextureSamplerState> shadowSamplerState;
		RefCountedPtr<TextureSamplerState> pcfSamplerState;

		RefCountedPtr<HWVertexBuffer> fullScreenQuadVB;
		RefCountedPtr<HWIndexBuffer> fullScreenQuadIB;

		GLuint gBufferFbo;
		RefCountedPtr<HWRenderTexture2D> gbuffer0;
		RefCountedPtr<HWDepthStencilSurface> sceneDepthMap;
        
		GLuint shadowAccumFbo;
		RefCountedPtr<HWRenderTexture2D> sceneShadowAccumMap;

		GLuint shadowMapFbo;
		RefCountedPtr<HWDepthStencilSurface> shadowMap;
		RefCountedPtr<HWRenderTexture2D> varianceShadowMap;
        RefCountedPtr<HWRenderTexture2D> bluredVarianceShadowMap;

		GLuint gaussianBlurFbo;
		
		GLRendererOptions options;
	};

    RendererGL::RendererGL(RenderDevice* _device) : device(_device), wireframeShader(0), matShader(0)
    {
        
		ReloadShaders();
        
		whiteTexture = GenerateColorTexture(1.f, 1.f, 1.f, 0.f);
		blackTexture = GenerateColorTexture(0.f, 0.f, 0.f, 0.f);
		uniZTexture = GenerateColorTexture(0.5f, 0.5f, 1.f, 0.f);
		noiseTexture = GenerateNoiseTexture(64, 64);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_TEXTURE_2D);

		TextureSamplerState* samplerState = NULL;

		{
			TextureSamplerState::Desc defaultSamplerStateDesc;
			defaultSamplerStateDesc.filter = TextureSamplerState::TEX_FILTER_ANISOTROPIC;
			defaultSamplerStateDesc.maxAnisotropy = 8;
			defaultSamplerStateDesc.uAddressMode = TextureSamplerState::TEX_ADDRESS_WRAP;
			defaultSamplerStateDesc.vAddressMode = TextureSamplerState::TEX_ADDRESS_WRAP;
			defaultSamplerStateDesc.wAddressMode = TextureSamplerState::TEX_ADDRESS_WRAP;
			device->CreateSamplerState(&defaultSamplerStateDesc, &samplerState);
			defaultSamplerState = samplerState;
		}

		{
			TextureSamplerState::Desc mipMapClampSamplerStateDesc;
			mipMapClampSamplerStateDesc.filter = TextureSamplerState::TEX_FILTER_MIN_MAG_MIP_LINEAR;
			mipMapClampSamplerStateDesc.maxAnisotropy = 0;
			mipMapClampSamplerStateDesc.uAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			mipMapClampSamplerStateDesc.vAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			mipMapClampSamplerStateDesc.wAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			device->CreateSamplerState(&mipMapClampSamplerStateDesc, &samplerState);
			mipMapClampSamplerState = samplerState;
				
		}

		{
			TextureSamplerState::Desc pointSamplerStateDesc;
			pointSamplerStateDesc.filter = TextureSamplerState::TEX_FILTER_MIN_MAG_MIP_POINT;
			pointSamplerStateDesc.maxAnisotropy = 0;
			pointSamplerStateDesc.uAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			pointSamplerStateDesc.vAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			pointSamplerStateDesc.wAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			device->CreateSamplerState(&pointSamplerStateDesc, &samplerState);
			pointSamplerState = samplerState;
		}

		{
			TextureSamplerState::Desc pcfSamplerStateDesc;
			pcfSamplerStateDesc.filter = TextureSamplerState::TEX_FILTER_PCF_SHADOW_MAP;
			pcfSamplerStateDesc.comparisonFunc = TextureSamplerState::TEX_COMPARE_LESS;
			pcfSamplerStateDesc.uAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			pcfSamplerStateDesc.vAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			pcfSamplerStateDesc.wAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			device->CreateSamplerState(&pcfSamplerStateDesc, &samplerState);
			pcfSamplerState = samplerState;
		}

		{
			TextureSamplerState::Desc shadowSamplerStateDesc;
			shadowSamplerStateDesc.filter = TextureSamplerState::TEX_FILTER_SHADOW_MAP;
			shadowSamplerStateDesc.comparisonFunc = TextureSamplerState::TEX_COMPARE_LESS;
			shadowSamplerStateDesc.uAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			shadowSamplerStateDesc.vAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			shadowSamplerStateDesc.wAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
			device->CreateSamplerState(&shadowSamplerStateDesc, &samplerState);
			shadowSamplerState = samplerState;
		}

        //create shadow map
		{
			HWTexture2D::Desc texDesc;
			texDesc.arraySize = 1;
			texDesc.format = TEX_FORMAT_DEPTH32F;
			texDesc.width = 512;
			texDesc.height = 512;
			texDesc.mipLevels = 1;
			texDesc.generateMipmap =false;
		
			HWRenderTexture2D::Desc rtDesc;
			rtDesc.format = TEX_FORMAT_DEPTH32F;
			rtDesc.mipLevel = 0;

			HWDepthStencilSurface* dsSurface = NULL;
			HWDepthStencilSurface::Desc desc;
			desc.format = TEX_FORMAT_DEPTH32F;
			desc.mipLevel = 0;
			device->CreateDepthStencilSurface(&texDesc, &desc, &dsSurface);
			shadowMap = dsSurface;


			//create variance shadow map
			texDesc.format = TEX_FORMAT_RG16F;
			rtDesc.mipLevel = 0;
			texDesc.mipLevels = TotalMipLevels(texDesc.width, texDesc.height);
			HWRenderTexture2D* rtVarianceShadowMap = NULL;
			//HWTexture2D* rtShadowMap = NULL;
			device->CreateRenderTexture2D(&texDesc, &rtDesc, &rtVarianceShadowMap);
			varianceShadowMap = rtVarianceShadowMap;
			
			
			//create temp blured version
			texDesc.mipLevels = 1;
			device->CreateRenderTexture2D(&texDesc, &rtDesc, &rtVarianceShadowMap);
			bluredVarianceShadowMap = rtVarianceShadowMap;
			
		}

		glGenFramebuffers(1, &shadowMapFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap->GetTexture()->GetImpl()->id, shadowMap->GetDesc()->mipLevel);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, varianceShadowMap->GetTexture()->GetImpl()->id, varianceShadowMap->GetDesc()->mipLevel);
		
		GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
		{

			printf("shadow map framebuffer incomplete\n");
		}
		
		glGenFramebuffers(1, &gaussianBlurFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, gaussianBlurFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bluredVarianceShadowMap->GetTexture()->GetImpl()->id, bluredVarianceShadowMap->GetDesc()->mipLevel);
		fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		
		if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("blur variance framebuffer incomplete\n");
		}
		
		
        //create g-buffer
		{
			HWTexture2D::Desc texDesc;
			texDesc.arraySize = 1;
			texDesc.format = TEX_FORMAT_RGBA16F;
			texDesc.width = device->window->width;
			texDesc.height = device->window->height;
			texDesc.mipLevels = 1;
			texDesc.generateMipmap =false;
			
            HWRenderTexture2D::Desc rtDesc;
            rtDesc.format = TEX_FORMAT_RGBA16F;
            rtDesc.mipLevel = 0;
            
            HWRenderTexture2D* rtPosMap = NULL;
			device->CreateRenderTexture2D(&texDesc, &rtDesc, &rtPosMap);
            gbuffer0 = rtPosMap;
		}
        
        {
			HWTexture2D::Desc texDesc;
			texDesc.arraySize = 1;
			texDesc.format = TEX_FORMAT_DEPTH32F;
			texDesc.width = device->window->width;
			texDesc.height = device->window->height;
			texDesc.mipLevels = 1;
			texDesc.generateMipmap =false;
            
			HWDepthStencilSurface* dsSurface = NULL;
			HWDepthStencilSurface::Desc desc;
			desc.format = TEX_FORMAT_DEPTH32F;
			desc.mipLevel = 0;
			device->CreateDepthStencilSurface(&texDesc, &desc, &dsSurface);
			sceneDepthMap = dsSurface;
        }
        
        glGenFramebuffers(1, &gBufferFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gbuffer0->GetTexture()->GetImpl()->id, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sceneDepthMap->GetTexture()->GetImpl()->id, 0);
        
		fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        
		if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
		{
            
			printf("g buffer framebuffer incomplete\n");
		}
        
        
        //create shadow accumulation buffer
        {
			HWTexture2D::Desc texDesc;
			texDesc.arraySize = 1;
			texDesc.format = TEX_FORMAT_R8;
			texDesc.width = device->window->width;
			texDesc.height = device->window->height;
			texDesc.mipLevels = 1;
			texDesc.generateMipmap =false;
			
            HWRenderTexture2D::Desc rtDesc;
            rtDesc.format = TEX_FORMAT_R8;
            rtDesc.mipLevel = 0;
            
            HWRenderTexture2D* rtShadowAccumMap = NULL;
			device->CreateRenderTexture2D(&texDesc, &rtDesc, &rtShadowAccumMap);
            sceneShadowAccumMap = rtShadowAccumMap;
        }
        glGenFramebuffers(1, &shadowAccumFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowAccumFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneShadowAccumMap->GetTexture()->GetImpl()->id, 0);
 		fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        
		if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
		{
            
			printf("shadow accum framebuffer incomplete\n");
		}

        
		VertexP3T2 quad[4];
		quad[0].position = Vector3(-1, -1, 0); quad[0].texcoord = Vector2(0, 0);
		quad[1].position = Vector3(1, -1, 0); quad[1].texcoord = Vector2(1, 0);
		quad[2].position = Vector3(-1, 1, 0); quad[2].texcoord = Vector2(0, 1);
		quad[3].position = Vector3(1, 1, 0); quad[3].texcoord = Vector2(1, 1);

		HWVertexBuffer* vb = NULL;

		device->CreateVertexBuffer(sizeof(quad), quad, &vb);
		fullScreenQuadVB = vb;
		

		int quadIndices[6] = {0, 1, 2, 2, 1, 3};
		HWIndexBuffer* ib = NULL;
		device->CreateIndexBuffer(sizeof(quadIndices), quadIndices, &ib);
		fullScreenQuadIB = ib;
        
        cubeVbo = CreateCubeVertexBuffer();
        cubeIbo = CreateWireCubeIndexBuffer();
	}

    RendererGL::~RendererGL()
    {
		glDeleteTextures(1, &whiteTexture);
		glDeleteTextures(1, &blackTexture);
		glDeleteTextures(1, &uniZTexture);
		glDeleteShader(wireframeShader);
		glDeleteShader(matShader);
    }

	static void TurnOffAllAttributes()
	{
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);
		glDisableVertexAttribArray(6);
		glDisableVertexAttribArray(7);
	}

    static void SetVertexAttributeP3N3T4T2(GLint posLoc, GLint normalLoc, GLint tangentLoc, GLint texcoordLoc)
    {
        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T4T2), 0);
        
        glEnableVertexAttribArray(normalLoc);
        glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T4T2), (GLvoid*)(sizeof(float) * 3));
        
        
        glEnableVertexAttribArray(tangentLoc);
        glVertexAttribPointer(tangentLoc, 4, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T4T2), (GLvoid*)(sizeof(float) * 6));
        
        
        glEnableVertexAttribArray(texcoordLoc);
        glVertexAttribPointer(texcoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T4T2), (GLvoid*)(sizeof(float) * 10));
    }
    
	static void SetTextureUnit(GLint texLoc, int activeTex, GLuint texID)
	{
		glUniform1i(texLoc, activeTex);
		glActiveTexture(GL_TEXTURE0 + activeTex);
		glBindTexture(GL_TEXTURE_2D, texID);
	}
    
    static void SetTextureUnit(GLint texLoc, int activeTex, const HWTexture2D* tex)
    {
		glUniform1i(texLoc, activeTex);
		glActiveTexture(GL_TEXTURE0 + activeTex);
        glBindTexture(GL_TEXTURE_2D, tex->GetImpl()->id);
    }

	void ShadowBound(const Scene* scene, AABB& outBound)
	{
		AABB bound;
		for(size_t primIdx = 0; primIdx < scene->primList.size(); primIdx++)
		{
			if(scene->primList[primIdx]->castShadow)
				bound = Merge(bound, scene->primList[primIdx]->WorldBound());
		}
		outBound = bound;
	}

    void RendererGL::RenderGBuffer(const jade::Camera *camera, const jade::Scene *scene)
    {
		glBindFramebuffer(GL_FRAMEBUFFER, gBufferFbo);
		glClearColor(1.0, 1.0, 1.0, 1);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
        
		glUseProgram(gbufferShader);
		GLint positionAttributeLoc = glGetAttribLocation(gbufferShader, "position");
		GLint normalAttributeLoc = glGetAttribLocation(gbufferShader, "normal");
		GLint tangentAttributeLoc = glGetAttribLocation(gbufferShader, "tangent");
		GLint texAttributeLoc = glGetAttribLocation(gbufferShader, "texcoord");
    
		GLint modelMatLoc = glGetUniformLocation(gbufferShader, "modelMatrix");
		GLint viewMatLoc = glGetUniformLocation(gbufferShader, "viewMatrix");
		GLint projectionMatLoc = glGetUniformLocation(gbufferShader, "projectionMatrix");
        
		Matrix4x4 viewMatrix = camera->ViewMatrix();
		Matrix4x4 projectionMatrix = camera->PerspectiveMatrix();
		glUniformMatrix4fv(viewMatLoc, 1, GL_TRUE, viewMatrix.FloatPtr());
		glUniformMatrix4fv(projectionMatLoc, 1, GL_TRUE, projectionMatrix.FloatPtr() );
        
		glViewport(0, 0, (GLsizei) camera->width , (GLsizei) camera->height);
        
        for(size_t primIdx = 0; primIdx < scene->primList.size(); primIdx++)
        {
            const Primitive* prim = scene->primList[primIdx].Get();
			
            glUniformMatrix4fv(modelMatLoc, 1, GL_TRUE, prim->ModelMatrix().FloatPtr());
            
            
            glBindBuffer(GL_ARRAY_BUFFER, prim->mesh->vertexBuffer->GetImpl()->vboID);
            SetVertexAttributeP3N3T4T2(positionAttributeLoc, normalAttributeLoc, tangentAttributeLoc, texAttributeLoc);
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->mesh->indexBuffer->GetImpl()->iboID);
           
            
            glDrawElements(GL_TRIANGLES, prim->mesh->indexBuffer->IndexCount(), GL_UNSIGNED_INT, 0);
        }
        
    }
    
    void RendererGL::RenderShadowMap(const jade::PointLight *light, const jade::Scene *scene, const Camera* cam)
    {
		if(options.shadowTech == GLRendererOptions::SHADOW_MAP_PCF)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, this->shadowMapFbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		}
		else if(options.shadowTech == GLRendererOptions::SHADOW_VARIANCE_SHADOW_MAP)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, this->shadowMapFbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->varianceShadowMap->GetTexture()->GetImpl()->id, this->varianceShadowMap->GetDesc()->mipLevel);
		
			GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
			{

				printf("shadow map framebuffer incomplete\n");
			}
		}

		glDisable(GL_BLEND);
	
		GLint modelMatLoc = glGetUniformLocation(shadowCasterPointLightShader, "modelMatrix");
		GLint shadowDepthMatLoc = glGetUniformLocation(shadowCasterPointLightShader, "shadowDepthMatrix");
		GLint shadowMapMatrixLoc = glGetUniformLocation(shadowCasterPointLightShader, "shadowMapMatrix");
		GLint minDepthLoc = glGetUniformLocation(shadowCasterPointLightShader, "minDepth");
		GLint maxDepthLoc = glGetUniformLocation(shadowCasterPointLightShader, "maxDepth");

		for(int i = 0; i < 6; i++)
		//int i = 0;
		{
			glUseProgram(shadowCasterPointLightShader);
			glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);
			
			glDepthFunc(GL_LESS);
			glViewport(0, 0, shadowMap->GetTexture()->GetDesc()->width, shadowMap->GetTexture()->GetDesc()->height);
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClearDepth(1.0f);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


			Matrix4x4 shadowViewMatrix = light->ShadowViewMatrix(i);
			Matrix4x4 shadowDepthMatrix = Matrix4x4(1, 0, 0, 0,
													0, 1, 0, 0,
													0, 0, -1, 0,
													0, 0, 0, 1) * 
				shadowViewMatrix;



			Matrix4x4 shadowProjMatrix = light->ShadowProjMatrix();

			float minDepth = 0.1f;
			float maxDepth = 3000.f;

			glUniform1f(minDepthLoc, minDepth);
			glUniform1f(maxDepthLoc, maxDepth);

			Matrix4x4 shadowMapMatrix = shadowProjMatrix * shadowViewMatrix;
			for(size_t primIdx = 0; primIdx < scene->primList.size(); primIdx++)
			{
				if(scene->primList[primIdx]->castShadow )
				{
					const Primitive* prim = scene->primList[primIdx].Get();
					glUniformMatrix4fv(modelMatLoc, 1, GL_TRUE, prim->ModelMatrix().FloatPtr());
					glUniformMatrix4fv(shadowDepthMatLoc, 1, GL_TRUE, shadowDepthMatrix.FloatPtr());
					glUniformMatrix4fv(shadowMapMatrixLoc, 1, GL_TRUE, shadowMapMatrix.FloatPtr() );

					glBindBuffer(GL_ARRAY_BUFFER, prim->mesh->vertexBuffer->GetImpl()->vboID);
					glEnableVertexAttribArray(0);

					SetVertexAttributeP3N3T4T2(0, 1, -1, -1);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->mesh->indexBuffer->GetImpl()->iboID);

					glDrawElements(GL_TRIANGLES, prim->mesh->indexBuffer->IndexCount(), GL_UNSIGNED_INT, 0);
				}
			}

			Matrix4x4 shadowMat = Matrix4x4(0.5, 0, 0, 0.5,
				0, 0.5, 0, 0.5,
				0, 0, 0.5, 0.5,
				0, 0, 0, 1) * shadowMapMatrix;

			AccumDeferredShadow(shadowMat, minDepth, maxDepth, shadowDepthMatrix, cam);
		}


    }
    
	void RendererGL::RenderShadowMap(const DirectionLight* light, const AABB& bound,  const Scene* scene, const Camera* cam)
	{
		if(options.shadowTech == GLRendererOptions::SHADOW_MAP_PCF)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, this->shadowMapFbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		}
		else if(options.shadowTech == GLRendererOptions::SHADOW_VARIANCE_SHADOW_MAP)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, this->shadowMapFbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->varianceShadowMap->GetTexture()->GetImpl()->id, this->varianceShadowMap->GetDesc()->mipLevel);
		
			GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
			{

				printf("shadow map framebuffer incomplete\n");
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, this->shadowMapFbo);

		glViewport(0, 0, shadowMap->GetTexture()->GetDesc()->width, shadowMap->GetTexture()->GetDesc()->height);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		glUseProgram(shadowCasterShader);

		GLint modelMatLoc = glGetUniformLocation(shadowCasterShader, "modelMatrix");
		GLint shadowMapMatrixLoc = glGetUniformLocation(shadowCasterShader, "shadowMapMatrix");
		GLint shadowDepthMatLoc = glGetUniformLocation(shadowCasterPointLightShader, "shadowDepthMatrix");
		GLint minDepthLoc = glGetUniformLocation(shadowCasterPointLightShader, "minDepth");
		GLint maxDepthLoc = glGetUniformLocation(shadowCasterPointLightShader, "maxDepth");


		Matrix4x4 shadowViewMatrix = light->ShadowViewMatrix();
		Matrix4x4 shadowMapMatrix = light->ShadowProjMatrix(bound) * light->ShadowViewMatrix();

		glUniformMatrix4fv(shadowMapMatrixLoc, 1, GL_TRUE, shadowMapMatrix.FloatPtr() );
		glUniformMatrix4fv(shadowDepthMatLoc, 1, GL_TRUE, shadowMapMatrix.FloatPtr() );

		glUniform1f(minDepthLoc, 0);
		glUniform1f(maxDepthLoc, 1);

		for(size_t primIdx = 0; primIdx < scene->primList.size(); primIdx++)
		{
			if(scene->primList[primIdx]->castShadow )
			{
				const Primitive* prim = scene->primList[primIdx].Get();
				glUniformMatrix4fv(modelMatLoc, 1, GL_TRUE, prim->ModelMatrix().FloatPtr());


				glBindBuffer(GL_ARRAY_BUFFER, prim->mesh->vertexBuffer->GetImpl()->vboID);
				glEnableVertexAttribArray(0);
                
                SetVertexAttributeP3N3T4T2(0, 1, -1, -1);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->mesh->indexBuffer->GetImpl()->iboID);

				glDrawElements(GL_TRIANGLES, prim->mesh->indexBuffer->IndexCount(), GL_UNSIGNED_INT, 0);
			}
		}

        Matrix4x4 shadowMat = Matrix4x4(0.5, 0, 0, 0.5,
                                          0, 0.5, 0, 0.5,
                                          0, 0, 0.5, 0.5,
                                          0, 0, 0, 1) * shadowMapMatrix;
        
        AccumDeferredShadow(shadowMat, 0, 1, shadowMat, cam);
		
	}

	void RendererGL::ClearDeferredShadow()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, shadowAccumFbo);
		glViewport(0, 0, sceneShadowAccumMap->GetTexture()->GetDesc()->width, sceneShadowAccumMap->GetTexture()->GetDesc()->height);

		glClearColor(1.f, 1.f, 1.f, 1.f);

		glClear(GL_COLOR_BUFFER_BIT);
	}

    void RendererGL::AccumDeferredShadow(const Matrix4x4& shadowMapMat, float minDepth, float maxDepth, const Matrix4x4& shadowDepthMatrix, const Camera* cam)
    {

		
		if(options.shadowTech == GLRendererOptions::SHADOW_MAP_PCF)
		{
			
		}
		else if(options.shadowTech == GLRendererOptions::SHADOW_VARIANCE_SHADOW_MAP)
		{
			GaussianBlur(varianceShadowMap, bluredVarianceShadowMap);

			glBindTexture(GL_TEXTURE_2D, varianceShadowMap->GetTexture()->GetImpl()->id);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, shadowAccumFbo);
		glViewport(0, 0, sceneShadowAccumMap->GetTexture()->GetDesc()->width, sceneShadowAccumMap->GetTexture()->GetDesc()->height);

		glEnable(GL_BLEND);
		glBlendEquation(GL_MIN);
		glBlendFunc(GL_ONE, GL_ONE);
        glDepthFunc(GL_ALWAYS);

		glUseProgram(deferredShadowShader);
		GLint shadowTechLoc = glGetUniformLocation(deferredShadowShader, "shadowTech");
		glUniform1i(shadowTechLoc, options.shadowTech);

		GLint gbuffer0Loc = glGetUniformLocation(deferredShadowShader, "gbuffer0");
		GLint depthMapLoc = glGetUniformLocation(deferredShadowShader, "sceneDepthMap");
		GLint noiseMapLoc = glGetUniformLocation(deferredShadowShader, "noiseMap");
		GLint invViewProjMatLoc = glGetUniformLocation(deferredShadowShader, "invViewProjMatrix");
		GLint shadowMapLoc = glGetUniformLocation(deferredShadowShader, "shadowMap");
		GLint singleSampleShadowMapLoc = glGetUniformLocation(deferredShadowShader, "singleSampleShadowMap");
		GLint varianceShadowMapLoc = glGetUniformLocation(deferredShadowShader, "varianceShadowMap");
		GLint shadowDepthMatrixLoc = glGetUniformLocation(deferredShadowShader, "shadowDepthMatrix");
		GLint shadowMatLoc = glGetUniformLocation(deferredShadowShader, "shadowMapMatrix");
		GLint minDepthLoc = glGetUniformLocation(deferredShadowShader, "minDepth");
		GLint maxDepthLoc = glGetUniformLocation(deferredShadowShader, "maxDepth");


		glUniformMatrix4fv(shadowMatLoc, 1, GL_TRUE, shadowMapMat.FloatPtr());
		glUniformMatrix4fv(shadowDepthMatrixLoc, 1, GL_TRUE,  shadowDepthMatrix.FloatPtr());
		glUniform1f(minDepthLoc, minDepth);
		glUniform1f(maxDepthLoc, maxDepth);


		Matrix4x4 invProjMatrix = cam->InvPerspectiveMatrix();
		Matrix4x4 test = invProjMatrix * cam->PerspectiveMatrix();
		
		Matrix4x4 invViewMatrix = cam->InvViewMatrix();
		
		test  = invViewMatrix * cam->ViewMatrix();
		
		Matrix4x4 invViewProjMatrix = invViewMatrix * invProjMatrix;
		glUniformMatrix4fv(invViewProjMatLoc, 1, GL_TRUE, invViewProjMatrix.FloatPtr());
		
		SetTextureUnit(gbuffer0Loc, 0, gbuffer0->GetTexture() );
		SetTextureUnit(shadowMapLoc, 1, shadowMap->GetTexture() );

		//SetTextureUnit(shadowMapLoc, 1, varianceShadowMap->GetTexture() );
		SetTextureUnit(varianceShadowMapLoc, 2, varianceShadowMap->GetTexture());
		SetTextureUnit(depthMapLoc, 3, sceneDepthMap->GetTexture() );
		SetTextureUnit(noiseMapLoc, 4, noiseTexture);

		SetTextureUnit(singleSampleShadowMapLoc, 5, shadowMap->GetTexture());

		glBindSampler(0, pointSamplerState->GetImpl()->sampler);
		glBindSampler(1, pcfSamplerState->GetImpl()->sampler);
		glBindSampler(2, mipMapClampSamplerState->GetImpl()->sampler);
		//glBindSampler(2, pcfSamplerState->GetImpl()->sampler);
        glBindSampler(3, pointSamplerState->GetImpl()->sampler);

		glBindSampler(5, shadowSamplerState->GetImpl()->sampler);

		glBindBuffer(GL_ARRAY_BUFFER, this->fullScreenQuadVB->GetImpl()->vboID);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3T2), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexP3T2), (GLvoid*)(sizeof(float) * 3));
        
        
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->fullScreenQuadIB->GetImpl()->iboID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		SetTextureUnit(gbuffer0Loc, 0, (GLuint)0 );
		SetTextureUnit(shadowMapLoc, 1, (GLuint)0 ); //make sure input != output
		glDisable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		
    }
    
    void RendererGL::Render(const Camera* camera, const Scene* scene)
    {
		if(options.screenShot)
		{
			options.screenShot = false;
			//beware, recursive call...
			ScreenShot("jade.tga", camera, scene);
			return;
		}
		
		RenderGBuffer(camera, scene);
        
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_FRAMEBUFFER_SRGB);	

		glClearColor(0.1, 0.1, 0.1, 1);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLuint sceneShader = matShader;
        
		glUseProgram(sceneShader);
		TurnOffAllAttributes();
		GLint positionAttributeLoc = glGetAttribLocation(sceneShader, "position");
		GLint normalAttributeLoc = glGetAttribLocation(sceneShader, "normal");
		GLint tangentAttributeLoc = glGetAttribLocation(sceneShader, "tangent");
		GLint texAttributeLoc = glGetAttribLocation(sceneShader, "texcoord");

		GLint modelMatLoc = glGetUniformLocation(sceneShader, "modelMatrix");
		GLint invModelMatLoc = glGetUniformLocation(sceneShader, "invModelMatrix");
		GLint viewMatLoc = glGetUniformLocation(sceneShader, "viewMatrix");
		GLint projectionMatLoc = glGetUniformLocation(sceneShader, "projectionMatrix");
		GLint shadowMapMatrixLoc = glGetUniformLocation(sceneShader, "shadowMapMatrix");
		GLint windowSizeLoc = glGetUniformLocation(sceneShader, "window_size");
		glUniform2f(windowSizeLoc, camera->width * device->setting.screenScaleFactor, camera->height * device->setting.screenScaleFactor);
		
		GLint ambientLoc = glGetUniformLocation(sceneShader, "ambient");
		GLint diffuseLoc = glGetUniformLocation(sceneShader, "diffuse");
		GLint specularLoc = glGetUniformLocation(sceneShader, "specular");
		GLint roughnessLoc = glGetUniformLocation(sceneShader, "roughness");

		GLint camPosLocation = glGetUniformLocation(sceneShader, "world_cam_pos");
		GLint useTangentLightLoc = glGetUniformLocation(sceneShader, "useTangentLight");
		GLint useDeferredShadowLoc = glGetUniformLocation(sceneShader, "useDeferredShadow");
		
		GLint diffuseMapLoc = glGetUniformLocation(sceneShader, "diffuseMap");
		GLint normalMapLoc = glGetUniformLocation(sceneShader, "normalMap");
		GLint specularMapLoc = glGetUniformLocation(sceneShader, "specularMap");
		GLint maskMapLoc = glGetUniformLocation(sceneShader, "maskMap");
		GLint shadowMapLoc = glGetUniformLocation(sceneShader, "shadowMap");
		GLint useMaskLoc = glGetUniformLocation(sceneShader, "useMask");

		GLint lightPosDirLoc = glGetUniformLocation(sceneShader, "lightPosDir");
		GLint lightRadiusLoc = glGetUniformLocation(sceneShader, "lightRadius");
		GLint lightIntensityLoc = glGetUniformLocation(sceneShader, "lightIntensity");

		GLint dbgDrawModeLoc = glGetUniformLocation(sceneShader, "dbgShowMode");
        
		glUniform3fv(camPosLocation, 1, reinterpret_cast<const float*>(&camera->position) );
        
		Matrix4x4 viewMatrix = camera->ViewMatrix();
		Matrix4x4 projectionMatrix = camera->PerspectiveMatrix();
		glUniformMatrix4fv(viewMatLoc, 1, GL_TRUE, viewMatrix.FloatPtr());
		glUniformMatrix4fv(projectionMatLoc, 1, GL_TRUE, projectionMatrix.FloatPtr() );
        
		glUniform1i(dbgDrawModeLoc, options.dbgDraw);
        
		for(size_t lightIdx = 0; lightIdx < scene->lightList.size(); lightIdx++)
		{

			//make sure render texture is not bound to texture unit
			{
				SetTextureUnit(shadowMapLoc, 4, (GLuint)0);
			}

			const Light* light = scene->lightList[lightIdx];

			switch(light->type)
			{
			case Light::LT_POINT:
				{
					const PointLight* pointLight = static_cast<const PointLight*> (light);

					Vector4 lightPos(pointLight->pos.x, pointLight->pos.y, pointLight->pos.z, 1.f);

					glUniform4fv(lightPosDirLoc, 1, reinterpret_cast<const float*> (&lightPos) );
					glUniform3fv(lightIntensityLoc, 1, reinterpret_cast<const float*> (&pointLight->intensity) );
					glUniform1f(lightRadiusLoc, pointLight->radius);
					ClearDeferredShadow();
					RenderShadowMap(pointLight, scene, camera);

				}
				break;
			case Light::LT_DIRECTION:
				{
					const DirectionLight* dirLight = static_cast<const DirectionLight*> (light);

					Vector4 lightDir(dirLight->dir.x, dirLight->dir.y, dirLight->dir.z, 0.f);

					glUniform4fv(lightPosDirLoc, 1, reinterpret_cast<const float*> (&lightDir) );
					glUniform3fv(lightIntensityLoc, 1, reinterpret_cast<const float*> (&dirLight->intensity) );	

                    
					Matrix4x4 shadowVpMat = Matrix4x4(0.5, 0, 0, 0.5,
												  0, 0.5, 0, 0.5,
												  0, 0, 0.5, 0.5,
												  0, 0, 0, 1);
					AABB bound;
					ShadowBound(scene, bound);
					Matrix4x4 shadowMapMatrix = shadowVpMat * dirLight->ShadowProjMatrix(bound) * dirLight->ShadowViewMatrix();
                     
					glUniformMatrix4fv(shadowMapMatrixLoc, 1, GL_TRUE, shadowMapMatrix.FloatPtr());
					ClearDeferredShadow();
					RenderShadowMap(dirLight, bound, scene, camera);
				}
				break;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);


			if(lightIdx == 0 || (options.dbgDraw >= GLRendererOptions::DBG_DRAW_UV_TILING && options.dbgDraw <= GLRendererOptions::DBG_DRAW_SPECULAR) )
			{
				glDisable(GL_BLEND);
				glDepthFunc(GL_LESS);
			}
			else
			{
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_ONE, GL_ONE);
				glDepthFunc(GL_EQUAL);
			}
			glViewport(0, 0, (GLsizei) camera->width * device->setting.screenScaleFactor, (GLsizei) camera->height * device->setting.screenScaleFactor);
			glUseProgram(sceneShader);
			for(size_t primIdx = 0; primIdx < scene->primList.size(); primIdx++)
			{
				const Primitive* prim = scene->primList[primIdx].Get();
			
				glUniformMatrix4fv(modelMatLoc, 1, GL_TRUE, prim->ModelMatrix().FloatPtr());
				glUniformMatrix4fv(invModelMatLoc, 1, GL_TRUE, prim->InvModelMatrix().FloatPtr());
                

				glBindBuffer(GL_ARRAY_BUFFER, prim->mesh->vertexBuffer->GetImpl()->vboID);
				SetVertexAttributeP3N3T4T2(positionAttributeLoc, normalAttributeLoc, tangentAttributeLoc, texAttributeLoc);
             
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->mesh->indexBuffer->GetImpl()->iboID);
	
				glUniform3fv(diffuseLoc, 1, reinterpret_cast<const float*> (&prim->material->diffuse) );
				glUniform3fv(specularLoc, 1, reinterpret_cast<const float*> (&prim->material->specular) );
				glUniform1f(roughnessLoc, prim->material->roughness);

				glBindSampler(0, defaultSamplerState->GetImpl()->sampler);
				glBindSampler(1, defaultSamplerState->GetImpl()->sampler);
				glBindSampler(2, defaultSamplerState->GetImpl()->sampler);
				glBindSampler(3, defaultSamplerState->GetImpl()->sampler);

				if(prim->material->diffuseMap)
				{
					SetTextureUnit(diffuseMapLoc, 0, prim->material->diffuseMap->hwTexture.Get());
				}
				else
				{
					SetTextureUnit(diffuseMapLoc, 0, whiteTexture);
				}

				if(prim->material->normalMap )
				{
					SetTextureUnit(normalMapLoc, 1, prim->material->normalMap->hwTexture.Get());
					glUniform1i(useTangentLightLoc, 1);
				}
				else
				{
					SetTextureUnit(normalMapLoc, 1, uniZTexture);
					glUniform1i(useTangentLightLoc, 0);
				}

				
				if(prim->material->specularMap )
				{
					SetTextureUnit(specularMapLoc, 2, prim->material->specularMap->hwTexture.Get());
				}
				else
				{
					SetTextureUnit(specularMapLoc, 2, blackTexture);
					
				}
				
				if(prim->material->dissolveMask)
				{
					SetTextureUnit(maskMapLoc, 3, prim->material->dissolveMask->hwTexture.Get());
					glUniform1i(useMaskLoc, 1);
				}
				else
				{
					SetTextureUnit(maskMapLoc, 3, whiteTexture);
					glUniform1i(useMaskLoc, 0);
				}
				
				{
					glUniform1i(useDeferredShadowLoc, 1);
					glBindSampler(4, pointSamplerState->GetImpl()->sampler);
					SetTextureUnit(shadowMapLoc, 4, this->sceneShadowAccumMap->GetTexture());
				}

				glDrawElements(GL_TRIANGLES, prim->mesh->indexBuffer->IndexCount(), GL_UNSIGNED_INT, 0);
			}


		}

        
        //AABB bound;
       // ShadowBound(scene, bound);
       // DrawBoundingBox(camera, bound);
        
		if(options.dbgDraw == GLRendererOptions::DBG_DRAW_SHADOW_MAP)
		{
			DrawTexture(0, 0, 256, 256, this->varianceShadowMap->GetTexture());
			DrawTexture(0, 256, 256, 256, this->sceneShadowAccumMap->GetTexture());
			DrawLightBounding(camera, scene);
		}
		

		glDisable(GL_FRAMEBUFFER_SRGB);
        
    }

    void RendererGL::SetRendererOption(void * _options)
    {
        options = *(static_cast<GLRendererOptions*>(_options));
        
        if(options.reloadShaders)
        {
            ReloadShaders();
            options.reloadShaders = false;
        }
    }
    
    void RendererGL::ReloadShaders()
    {
        
		GLuint _wireframeShader =  CreateProgram("shader/wireVertexShader.glsl", "shader/wirePixelShader.glsl");
		
        if(_wireframeShader)
            wireframeShader = _wireframeShader;
        
        GLuint  _matShader = CreateProgram("shader/blinn_phong_vs.glsl", "shader/blinn_phong_ps.glsl");
        
        if(_matShader)
            matShader = _matShader;

		GLuint _blitShader = CreateProgram("shader/full_screen_vs.glsl", "shader/full_screen_blit_ps.glsl");
    
		if(_blitShader)
			blitShader = _blitShader;

		GLuint _shadowCasterShader = CreateProgram("shader/shadow_caster_vs.glsl", "shader/shadow_caster_ps.glsl");

		if(_shadowCasterShader)
			shadowCasterShader = _shadowCasterShader;
        
		GLuint _shadowCasterPointShader = CreateProgram("shader/shadow_caster_vs.glsl", "shader/shadow_caster_point_light_ps.glsl");
		
		if(_shadowCasterPointShader)
			shadowCasterPointLightShader = _shadowCasterPointShader;

        GLuint _gbufferShader = CreateProgram("shader/gbuffer_vs.glsl", "shader/gbuffer_ps.glsl");
        
        if(_gbufferShader)
            gbufferShader = _gbufferShader;
        
        GLuint _deferredShadowShader = CreateProgram("shader/full_screen_vs.glsl", "shader/deferred_shadow_ps.glsl");
        
        if(_deferredShadowShader)
            deferredShadowShader = _deferredShadowShader;
		
		GLuint _gaussianBlurShader = CreateProgram("shader/full_Screen_vs.glsl", "shader/gaussian_blur.glsl");
		
		if(_gaussianBlurShader)
			gaussianBlurShader = _gaussianBlurShader;
    }
    
    void RendererGL::DrawBoundingBox(const Camera* camera, const AABB& bound)
    {
        TurnOffAllAttributes();
        glUseProgram(wireframeShader);
        
        glDisable(GL_BLEND);
        glDepthFunc(GL_LESS);
        
        GLint positionAttributeLocation = glGetAttribLocation(wireframeShader, "position");
        GLint colorAttributeLocation = glGetAttribLocation(wireframeShader, "color");
        
        GLint modelMatLocation = glGetUniformLocation(wireframeShader, "modelMatrix");
        GLint viewMatLocation = glGetUniformLocation(wireframeShader, "viewMatrix");
        GLint projectionMatLocation = glGetUniformLocation(wireframeShader, "projectionMatrix");
        
        Matrix4x4 viewMatrix = camera->ViewMatrix();
        Matrix4x4 projectionMatrix = camera->PerspectiveMatrix();
        
        glUniformMatrix4fv(viewMatLocation, 1, GL_TRUE, viewMatrix.FloatPtr());
        glUniformMatrix4fv(projectionMatLocation, 1, GL_TRUE, projectionMatrix.FloatPtr() );
        
        
        Matrix4x4 modelMatrix = Translate(bound.center) * Scale(bound.radius);
        glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());
        
        
        glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
        glEnableVertexAttribArray(positionAttributeLocation);
        glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
        glEnableVertexAttribArray(colorAttributeLocation);
        glVertexAttribPointer(colorAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIbo);
        

        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        
    }
	
	void RendererGL::DrawLightBounding(const jade::Camera *camera, const jade::Scene *scene)
	{
		for(size_t i = 0; i < scene->lightList.size(); i++)
		{
			if(scene->lightList[i]->type == Light::LT_POINT)
			{
				PointLight* ptLight = static_cast<PointLight*>(scene->lightList[i].Get());
				AABB bound;
				bound.center = ptLight->pos;
				bound.radius = Vector3(ptLight->radius);
				DrawBoundingBox(camera, bound);
			}
		}
	}
    
    void RendererGL::DrawTexture(int x, int y, int width, int height, const jade::HWTexture2D *tex)
    {
        TurnOffAllAttributes();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(x * device->setting.screenScaleFactor, y * device->setting.screenScaleFactor, width * device->setting.screenScaleFactor, height * device->setting.screenScaleFactor);
        
        
        glDisable(GL_BLEND);
        glDepthFunc(GL_ALWAYS);
        
        glUseProgram(blitShader);
        glBindBuffer(GL_ARRAY_BUFFER, this->fullScreenQuadVB->GetImpl()->vboID);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3T2), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexP3T2), (GLvoid*)(sizeof(float) * 3));
        
        
        GLint sourceTexLoc = glGetUniformLocation(blitShader, "source");
        SetTextureUnit(sourceTexLoc, 0, tex);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->fullScreenQuadIB->GetImpl()->iboID);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    
	void RendererGL::GaussianBlur(jade::HWRenderTexture2D *src, jade::HWRenderTexture2D *tmp)
	{
        TurnOffAllAttributes();
        glBindFramebuffer(GL_FRAMEBUFFER, gaussianBlurFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmp->GetTexture()->GetImpl()->id, 0);
		glViewport(0, 0, src->GetTexture()->GetDesc()->width, src->GetTexture()->GetDesc()->height);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_BLEND);
        glDepthFunc(GL_ALWAYS);
		
		glUseProgram(gaussianBlurShader);
		
		
		GLint blurDirLoc = glGetUniformLocation(gaussianBlurShader, "direction");
		GLint sourceLoc = glGetUniformLocation(gaussianBlurShader, "source");
		
        glBindBuffer(GL_ARRAY_BUFFER, this->fullScreenQuadVB->GetImpl()->vboID);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3T2), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexP3T2), (GLvoid*)(sizeof(float) * 3));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->fullScreenQuadIB->GetImpl()->iboID);
		
		glBindSampler(0, pointSamplerState->GetImpl()->sampler);
		
		SetTextureUnit(sourceLoc, 0, src->GetTexture()->GetImpl()->id);
		glUniform1i(blurDirLoc, 0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		SetTextureUnit(sourceLoc, 0, tmp->GetTexture()->GetImpl()->id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src->GetTexture()->GetImpl()->id, src->GetDesc()->mipLevel);
		
		glUniform1i(blurDirLoc, 1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
	}
	
	void GammaCorrect(unsigned char* imgBuffer, int width, int height)
	{
		for(int i = 0; i < height; i++)
		{
			for(int j = 0; j < width * 4; j++)
			{
				float fval = ((float)imgBuffer[i * width * 4 + j] / 255.f);
				
				fval = powf(fval, 1.f / 2.2f);
				
				imgBuffer[i * width * 4 + j] = (unsigned char) (fval * 255.f);
			}
		}
	}
	
	void RendererGL::ScreenShot(const char *path, const jade::Camera *camera, const jade::Scene *scene)
	{
		Render(camera, scene);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		size_t imgSize = device->window->width * device->window->height * 4 * device->setting.screenScaleFactor * device->setting.screenScaleFactor;
		unsigned char* imgBuffer = new unsigned char[imgSize];
		
		glPixelStorei(GL_PACK_ALIGNMENT,1); //or glPixelStorei(GL_PACK_ALIGNMENT,4);
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_SKIP_ROWS, 0);
		glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
		
		glReadPixels(0, 0,  device->window->width * device->setting.screenScaleFactor, device->window->height * device->setting.screenScaleFactor, GL_RGBA, GL_UNSIGNED_BYTE, imgBuffer);
		
	//	GammaCorrect(imgBuffer, device->window->width * 2, device->window->height * 2);
		SaveTGA(path, imgBuffer, device->window->width * device->setting.screenScaleFactor, device->window->height * device->setting.screenScaleFactor);
		delete [] imgBuffer;
	}
	
    void InitRendererGL(RenderDevice* device, Renderer** renderer)
    {
		RendererGL* rendererGL = NULL;
		*renderer = rendererGL = new RendererGL(device);
    }

    void ShutdownRendererGL(Renderer* renderer)
    {
        delete renderer;
    }
}