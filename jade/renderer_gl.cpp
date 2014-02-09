#include "renderer_gl.h"
#include "render_device_gl.h"
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

        virtual void SetRendererOption(void*);

        void ReloadShaders();
        
		void RenderDebugInfo(const Camera* camera, const Scene* scene);

		class RenderDevice* device;
		GLuint wireframeShader;
		GLuint matShader;
        
		GLuint whiteTexture;
		GLuint blackTexture;
        GLuint blueTexture; //unit z
        
		RefCountedPtr<TextureSamplerState> defaultSamplerState;
        
        GLRendererOptions options;
	};

    RendererGL::RendererGL(RenderDevice* _device) : device(_device), wireframeShader(0), matShader(0)
    {
        
		ReloadShaders();
        
		whiteTexture = GenerateColorTexture(1.f, 1.f, 1.f, 0.f);
		blackTexture = GenerateColorTexture(0.f, 0.f, 0.f, 0.f);
		blueTexture = GenerateColorTexture(0.f, 0.f, 1.f, 0.f);

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
    }

    RendererGL::~RendererGL()
    {

    }

	void SetTextureUnit(GLint texLoc, int activeTex, GLuint texID)
	{
		glUniform1i(texLoc, activeTex);
		glActiveTexture(GL_TEXTURE0 + activeTex);
		glBindTexture(GL_TEXTURE_2D, texID);
	}

    void RendererGL::Render(const Camera* camera, const Scene* scene)
    {
		glClearColor(0.5, 0.5, 0.5, 1);
	

		//glClearColor(0.f, 0.f, 0.f, 0);
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

		GLint lightPosDirLoc = glGetUniformLocation(sceneShader, "lightPosDir");
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
			if(lightIdx == 0)
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

			const Light* light = scene->lightList[lightIdx];

			switch(light->type)
			{
			case Light::LT_POINT:
				{
					const PointLight* pointLight = static_cast<const PointLight*> (light);

					Vector4 lightPos(pointLight->pos.x, pointLight->pos.y, pointLight->pos.z, 1.f);

					glUniform4fv(lightPosDirLoc, 1, reinterpret_cast<const float*> (&lightPos) );
					glUniform3fv(lightIntensityLoc, 1, reinterpret_cast<const float*> (&pointLight->intensity) );
				}
				break;
			case Light::LT_DIRECTION:
				{
					const DirectionLight* pointLight = static_cast<const DirectionLight*> (light);

					Vector4 lightDir(pointLight->dir.x, pointLight->dir.y, pointLight->dir.z, 0.f);

					glUniform4fv(lightPosDirLoc, 1, reinterpret_cast<const float*> (&lightDir) );
					glUniform3fv(lightIntensityLoc, 1, reinterpret_cast<const float*> (&pointLight->intensity) );					
				}
				break;
			}

			for(size_t primIdx = 0; primIdx < scene->primList.size(); primIdx++)
			{
				const Primitive* prim = scene->primList[primIdx].Get();
			
				glUniformMatrix4fv(modelMatLoc, 1, GL_TRUE, prim->ModelMatrix().FloatPtr());
                glUniformMatrix4fv(invModelMatLoc, 1, GL_TRUE, prim->InvModelMatrix().FloatPtr());
                
				glBindBuffer(GL_ARRAY_BUFFER, prim->mesh->vertexBuffer2->GetImpl()->vboID);
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

				glBindSampler(0, defaultSamplerState->GetImpl()->sampler);
                
                
				if(prim->material->diffuseMap && prim->material->diffuseMap->hwTexture)
				{
					SetTextureUnit(diffuseMapLoc, 0, prim->material->diffuseMap->hwTexture->GetImpl()->id);
				}
				else
				{
					SetTextureUnit(diffuseMapLoc, 0, whiteTexture);
				}

				if(prim->material->normalMap && prim->material->normalMap->hwTexture)
				{
					SetTextureUnit(normalMapLoc, 1, prim->material->normalMap->hwTexture->GetImpl()->id);
					glUniform1i(useTangentLightLoc, 1);
				}
				else
				{
					SetTextureUnit(normalMapLoc, 1, blueTexture);
					glUniform1i(useTangentLightLoc, 0);
				}

				
				if(prim->material->specularMap && prim->material->specularMap->hwTexture)
				{
					SetTextureUnit(specularMapLoc, 2, prim->material->specularMap->hwTexture->GetImpl()->id);
				}
				else
				{
					SetTextureUnit(specularMapLoc, 2, blackTexture);
					
				}

				glDrawElements(GL_TRIANGLES, prim->mesh->indexBuffer->IndexCount(), GL_UNSIGNED_INT, 0);
			}
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