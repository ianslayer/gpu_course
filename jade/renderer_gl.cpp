#include "renderer_gl.h"
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

		virtual void RenderShadowMap(const DirectionLight* light, const AABB& bound,  const Scene* scene);

        virtual void SetRendererOption(void*);

        void ReloadShaders();
        
		void RenderDebugInfo(const Camera* camera, const Scene* scene);

		class RenderDevice* device;
		GLuint wireframeShader;
		GLuint matShader;
		GLuint blitShader;
		GLuint shadowCasterShader;
        
		GLuint whiteTexture;
		GLuint blackTexture;
        GLuint uniZTexture; //unit z
        
		RefCountedPtr<TextureSamplerState> defaultSamplerState;
        RefCountedPtr<TextureSamplerState> shadowSamplerState;

		RefCountedPtr<HWVertexBuffer> fullScreenQuadVB;
		RefCountedPtr<HWIndexBuffer> fullScreenQuadIB;

        RefCountedPtr<HWDepthStencilSurface> sceneDepthMap;

		GLuint shadowMapFbo;
		RefCountedPtr<HWDepthStencilSurface> shadowMap;
        RefCountedPtr<HWRenderTexture2D> shadowColorMap; //for debugging
        
        GLRendererOptions options;
	};

    RendererGL::RendererGL(RenderDevice* _device) : device(_device), wireframeShader(0), matShader(0)
    {
        
		ReloadShaders();
        
		whiteTexture = GenerateColorTexture(1.f, 1.f, 1.f, 0.f);
		blackTexture = GenerateColorTexture(0.f, 0.f, 0.f, 0.f);
		uniZTexture = GenerateColorTexture(0.5f, 0.5f, 1.f, 0.f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_TEXTURE_2D);

		TextureSamplerState* samplerState = NULL;
		TextureSamplerState::Desc defaultSamplerStateDesc;
		defaultSamplerStateDesc.filter = TextureSamplerState::TEX_FILTER_ANISOTROPIC;
		defaultSamplerStateDesc.maxAnisotropy = 8;
		defaultSamplerStateDesc.uAddressMode = TextureSamplerState::TEX_ADDRESS_WRAP;
		defaultSamplerStateDesc.vAddressMode = TextureSamplerState::TEX_ADDRESS_WRAP;
		defaultSamplerStateDesc.wAddressMode = TextureSamplerState::TEX_ADDRESS_WRAP;
		device->CreateSamplerState(&defaultSamplerStateDesc, &samplerState);
		defaultSamplerState = samplerState;

		TextureSamplerState::Desc shadowSamplerStateDesc;
		shadowSamplerStateDesc.filter = TextureSamplerState::TEX_FILTER_MIN_MAG_MIP_POINT;
		shadowSamplerStateDesc.maxAnisotropy = 0;
		shadowSamplerStateDesc.uAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
		shadowSamplerStateDesc.vAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
		shadowSamplerStateDesc.wAddressMode = TextureSamplerState::TEX_ADDRESS_CLAMP;
		device->CreateSamplerState(&shadowSamplerStateDesc, &samplerState);
		shadowSamplerState = samplerState;

		/*
		HWTexture2D* shadowColorTexture = NULL;
		{
			HWTexture2D::Desc desc;
			desc.arraySize =1;
			desc.format = TEX_FORMAT_SRGB8_ALPHA8;
			desc.width = 2048;
			desc.height = 2048;
			desc.mipLevels = 1;
			desc.generateMipmap = false;
			device->CreateTexture2D(&desc, NULL, &shadowColorTexture);
		}

		{
			HWRenderTexture2D::Desc desc;
			desc.format = TEX_FORMAT_SRGB8_ALPHA8;
			desc.mipLevel = 0;
			HWRenderTexture2D* rtTexture = NULL;
			device->CreateRenderTexture2D(shadowColorTexture, &desc, &rtTexture);
			shadowColorMap = rtTexture;
		}
		*/

		HWTexture2D* shadowTexure = NULL;
		{
			HWTexture2D::Desc desc;
			desc.arraySize = 1;
			desc.format = TEX_FORMAT_DEPTH32F;
			desc.width = 2048;
			desc.height = 2048;
			desc.mipLevels = 1;
			desc.generateMipmap =false;
			
			device->CreateTexture2D(&desc, NULL, &shadowTexure);
		}

		{
			HWDepthStencilSurface* dsSurface = NULL;
			HWDepthStencilSurface::Desc desc;
			desc.format = TEX_FORMAT_DEPTH32F;
			desc.mipLevel = 0;
			device->CreateDepthStencilSurface(shadowTexure, &desc, &dsSurface);
			shadowMap = dsSurface;
		}

		glGenFramebuffers(1, &shadowMapFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowColorMap->GetTexture()->GetImpl()->id, shadowColorMap->GetDesc()->mipLevel);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap->GetTexture()->GetImpl()->id, shadowMap->GetDesc()->mipLevel);
			
		GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
		{

			printf("framebuffer incomplete\n");
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
	}

    RendererGL::~RendererGL()
    {
		glDeleteTextures(1, &whiteTexture);
		glDeleteTextures(1, &blackTexture);
		glDeleteTextures(1, &uniZTexture);
		glDeleteShader(wireframeShader);
		glDeleteShader(matShader);
    }

	void TurnOffAllAttributes()
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

	void SetTextureUnit(GLint texLoc, int activeTex, GLuint texID)
	{
		glUniform1i(texLoc, activeTex);
		glActiveTexture(GL_TEXTURE0 + activeTex);
		glBindTexture(GL_TEXTURE_2D, texID);
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

	void RendererGL::RenderShadowMap(const DirectionLight* light, const AABB& bound,  const Scene* scene)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);
		glUseProgram(shadowCasterShader);

		GLint diffuseMapLoc = glGetUniformLocation(shadowCasterShader, "diffuseMap");
		GLint modelMatLoc = glGetUniformLocation(shadowCasterShader, "modelMatrix");
		GLint shadowMapMatrixLoc = glGetUniformLocation(shadowCasterShader, "shadowMapMatrix");

		glClearColor(0.0, 0.1, 0.0, 1);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		glViewport(0, 0, 2048, 2048);


		Matrix4x4 shadowMapMatrix = light->ShadowProjMatrix(bound) * light->ShadowViewMatrix();
		int i = 0;
		for(size_t primIdx = 0; primIdx < scene->primList.size(); primIdx++)
		{
			if(scene->primList[primIdx]->castShadow )
			{
				i++;
				const Primitive* prim = scene->primList[primIdx].Get();
				glUniformMatrix4fv(modelMatLoc, 1, GL_TRUE, prim->ModelMatrix().FloatPtr());
				glUniformMatrix4fv(shadowMapMatrixLoc, 1, GL_TRUE, shadowMapMatrix.FloatPtr() );

				if(prim->material->diffuseMap)
				{
					SetTextureUnit(diffuseMapLoc, 0, prim->material->diffuseMap->hwTexture->GetImpl()->id);
				}
				else
				{
					SetTextureUnit(diffuseMapLoc, 0, whiteTexture);
				}

				glBindBuffer(GL_ARRAY_BUFFER, prim->mesh->vertexBuffer->GetImpl()->vboID);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T4T2), 0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T4T2), (GLvoid*)(sizeof(float) * 10));
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->mesh->indexBuffer->GetImpl()->iboID);

				glDrawElements(GL_TRIANGLES, prim->mesh->indexBuffer->IndexCount(), GL_UNSIGNED_INT, 0);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
	}

    void RendererGL::Render(const Camera* camera, const Scene* scene)
    {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_FRAMEBUFFER_SRGB);	

		glClearColor(0.1, 0.1, 0.1, 1);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLuint sceneShader = matShader;
        
		glUseProgram(sceneShader);
		GLint positionAttributeLoc = glGetAttribLocation(sceneShader, "position");
		GLint normalAttributeLoc = glGetAttribLocation(sceneShader, "normal");
        GLint tangentAttributeLoc = glGetAttribLocation(sceneShader, "tangent");
		GLint texAttributeLoc = glGetAttribLocation(sceneShader, "texcoord");

		GLint modelMatLoc = glGetUniformLocation(sceneShader, "modelMatrix");
        GLint invModelMatLoc = glGetUniformLocation(sceneShader, "invModelMatrix");
		GLint viewMatLoc = glGetUniformLocation(sceneShader, "viewMatrix");
		GLint projectionMatLoc = glGetUniformLocation(sceneShader, "projectionMatrix");
		GLint shadowMapMatrixLoc = glGetUniformLocation(sceneShader, "shadowMapMatrix");

		GLint ambientLoc = glGetUniformLocation(sceneShader, "ambient");
		GLint diffuseLoc = glGetUniformLocation(sceneShader, "diffuse");
		GLint specularLoc = glGetUniformLocation(sceneShader, "specular");
		GLint roughnessLoc = glGetUniformLocation(sceneShader, "roughness");

		GLint camPosLocation = glGetUniformLocation(sceneShader, "world_cam_pos");
		GLint useTangentLightLoc = glGetUniformLocation(sceneShader, "useTangentLight");

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

					glBindSampler(4, shadowSamplerState->GetImpl()->sampler);
					SetTextureUnit(shadowMapLoc, 4, whiteTexture);
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
					glBindSampler(4, shadowSamplerState->GetImpl()->sampler);

					SetTextureUnit(shadowMapLoc, 4, this->shadowMap->GetTexture()->GetImpl()->id);
					RenderShadowMap(dirLight, bound, scene);


				}
				break;
			}

			if(lightIdx == 0 || (options.dbgDraw >= GLRendererOptions::DBG_DRAW_UV_TILING && options.dbgDraw <= GLRendererOptions::DBG_DRAW_SPECULAR) )
			{
				glDisable(GL_BLEND);
				glDepthFunc(GL_LESS);
			}
			else
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
				glDepthFunc(GL_EQUAL);
			}
			glViewport(0, 0, camera->width, camera->height);
			glUseProgram(sceneShader);
			for(size_t primIdx = 0; primIdx < scene->primList.size(); primIdx++)
			{
				const Primitive* prim = scene->primList[primIdx].Get();
			
				glUniformMatrix4fv(modelMatLoc, 1, GL_TRUE, prim->ModelMatrix().FloatPtr());
                glUniformMatrix4fv(invModelMatLoc, 1, GL_TRUE, prim->InvModelMatrix().FloatPtr());
                

				glBindBuffer(GL_ARRAY_BUFFER, prim->mesh->vertexBuffer->GetImpl()->vboID);
				glEnableVertexAttribArray(positionAttributeLoc);
				glVertexAttribPointer(positionAttributeLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T4T2), 0);
                
				glEnableVertexAttribArray(normalAttributeLoc);
				glVertexAttribPointer(normalAttributeLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T4T2), (GLvoid*)(sizeof(float) * 3));
                
                
				glEnableVertexAttribArray(tangentAttributeLoc);
				glVertexAttribPointer(tangentAttributeLoc, 4, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T4T2), (GLvoid*)(sizeof(float) * 6));
                
                
				glEnableVertexAttribArray(texAttributeLoc);
				glVertexAttribPointer(texAttributeLoc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T4T2), (GLvoid*)(sizeof(float) * 10));
                
             
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->mesh->indexBuffer->GetImpl()->iboID);
				Matrix4x4 modelMatrix = prim->ModelMatrix();
				glUniformMatrix4fv(modelMatLoc, 1, GL_TRUE, modelMatrix.FloatPtr());

	
				glUniform3fv(diffuseLoc, 1, reinterpret_cast<const float*> (&prim->material->diffuse) );
				glUniform3fv(specularLoc, 1, reinterpret_cast<const float*> (&prim->material->specular) );
				glUniform1f(roughnessLoc, prim->material->roughness);

				glBindSampler(0, defaultSamplerState->GetImpl()->sampler);
                glBindSampler(1, defaultSamplerState->GetImpl()->sampler);
                glBindSampler(2, defaultSamplerState->GetImpl()->sampler);
				glBindSampler(3, defaultSamplerState->GetImpl()->sampler);

				if(prim->material->diffuseMap)
				{
					SetTextureUnit(diffuseMapLoc, 0, prim->material->diffuseMap->hwTexture->GetImpl()->id);
				}
				else
				{
					SetTextureUnit(diffuseMapLoc, 0, whiteTexture);
				}

				if(prim->material->normalMap )
				{
					SetTextureUnit(normalMapLoc, 1, prim->material->normalMap->hwTexture->GetImpl()->id);
					glUniform1i(useTangentLightLoc, 1);
				}
				else
				{
					SetTextureUnit(normalMapLoc, 1, uniZTexture);
					glUniform1i(useTangentLightLoc, 0);
				}

				
				if(prim->material->specularMap )
				{
					SetTextureUnit(specularMapLoc, 2, prim->material->specularMap->hwTexture->GetImpl()->id);
				}
				else
				{
					SetTextureUnit(specularMapLoc, 2, blackTexture);
					
				}

				
				if(prim->material->dissolveMask)
				{
					SetTextureUnit(maskMapLoc, 3, prim->material->dissolveMask->hwTexture->GetImpl()->id);
					glUniform1i(useMaskLoc, 1);
				}
				else
				{
					SetTextureUnit(maskMapLoc, 3, whiteTexture);
					glUniform1i(useMaskLoc, 0);
				}

				glDrawElements(GL_TRIANGLES, prim->mesh->indexBuffer->IndexCount(), GL_UNSIGNED_INT, 0);
			}
		}

		if(options.dbgDraw == GLRendererOptions::DBG_DRAW_SHADOW_MAP)
		{
			TurnOffAllAttributes();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, 256, 256);
			

			glDisable(GL_BLEND);
			glDepthFunc(GL_ALWAYS);

			glUseProgram(blitShader);
			glBindBuffer(GL_ARRAY_BUFFER, this->fullScreenQuadVB->GetImpl()->vboID);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3T2), 0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexP3T2), (GLvoid*)(sizeof(float) * 3));                


			GLint sourceTexLoc = glGetUniformLocation(blitShader, "source");
			SetTextureUnit(sourceTexLoc, 0, this->shadowMap->GetTexture()->GetImpl()->id);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->fullScreenQuadIB->GetImpl()->iboID);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		
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