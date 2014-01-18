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
		RendererGL();
		virtual ~RendererGL();

		virtual void Render(const Camera* camera, const Scene* scene);

		void RenderDebugInfo(const Camera* camera, const Scene* scene);

		class RenderDevice* device;
		GLuint wireframeShader;
		GLuint matShader;
		GLuint whiteTexture;
		GLuint blackTexture;
	};

    RendererGL::RendererGL() : wireframeShader(0)
    {
		wireframeShader =  CreateProgram("shader/wireVertexShader.glsl", "shader/wirePixelShader.glsl");
		matShader = CreateProgram("shader/blinn_phong_vs.glsl", "shader/blinn_phong_ps.glsl");

		whiteTexture = GenerateColorTexture(1.f, 1.f, 1.f, 1.f);
		blackTexture = GenerateColorTexture(0.f, 0.f, 0.f, 0.f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_TEXTURE_2D);
    }

    RendererGL::~RendererGL()
    {

    }

    void RendererGL::Render(const Camera* camera, const Scene* scene)
    {
		glClearColor(0.5, 0.5, 0.5, 1);
	

		glClearColor(0.f, 0.f, 0.f, 0);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(matShader);
		GLint positionAttributeLoc = glGetAttribLocation(matShader, "position");
		GLint normalAttributeLoc = glGetAttribLocation(matShader, "normal");
		GLint texAttributeLoc = glGetAttribLocation(matShader, "texcoord");

		GLint modelMatLoc = glGetUniformLocation(matShader, "modelMatrix");
		GLint viewMatLoc = glGetUniformLocation(matShader, "viewMatrix");
		GLint projectionMatLoc = glGetUniformLocation(matShader, "projectionMatrix");

		GLint ambientLoc = glGetUniformLocation(matShader, "ambient");
		GLint diffuseLoc = glGetUniformLocation(matShader, "diffuse");
		GLint specularLoc = glGetUniformLocation(matShader, "specular");
		GLint roughnessLoc = glGetUniformLocation(matShader, "roughness");

		GLint camPosLocation = glGetUniformLocation(matShader, "cam_pos");

		GLint diffuseTexLoc = glGetUniformLocation(matShader, "diffuseTex"); 
		
		GLint lightPosDirLoc = glGetUniformLocation(matShader, "lightPosDir");
		GLint lightIntensityLoc = glGetUniformLocation(matShader, "lightIntensity");

		Matrix4x4 viewMatrix = camera->ViewMatrix();
		Matrix4x4 projectionMatrix = camera->PerspectiveMatrix();
		glUniformMatrix4fv(viewMatLoc, 1, GL_TRUE, viewMatrix.FloatPtr());
		glUniformMatrix4fv(projectionMatLoc, 1, GL_TRUE, projectionMatrix.FloatPtr() );

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

				glBindBuffer(GL_ARRAY_BUFFER, prim->mesh->vertexBuffer->GetImpl()->vboID);
				glEnableVertexAttribArray(positionAttributeLoc);
				glVertexAttribPointer(positionAttributeLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T2), 0);

				glEnableVertexAttribArray(normalAttributeLoc);
				glVertexAttribPointer(normalAttributeLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T2), (GLvoid*)(sizeof(float) * 3));
			
				size_t vertSize = sizeof(VertexP3N3T2);
				glEnableVertexAttribArray(texAttributeLoc);
				glVertexAttribPointer(texAttributeLoc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T2), (GLvoid*)(sizeof(float) * 6));
			

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->mesh->indexBuffer->GetImpl()->iboID);
				Matrix4x4 modelMatrix = prim->ModelMatrix();
				glUniformMatrix4fv(modelMatLoc, 1, GL_TRUE, modelMatrix.FloatPtr());

	
				glUniform3fv(diffuseLoc, 1, reinterpret_cast<const float*> (&prim->material->diffuse) );
				glUniform3fv(specularLoc, 1, reinterpret_cast<const float*> (&prim->material->specular) );

				if(prim->material->diffuseMap && prim->material->diffuseMap->hwTexture)
				{
					glUniform1i(diffuseTexLoc, 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, prim->material->diffuseMap->hwTexture->GetImpl()->id);
				}
				else
				{
					glUniform1i(diffuseTexLoc, 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D,whiteTexture);
				}



				/*
				if(prim->material->normalMap && prim->material->normalMap->hwTexture)
				{
					glUniform1i(texLocation, 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, prim->material->normalMap->hwTexture->GetImpl()->id);
				}		 
				*/


				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDrawElements(GL_TRIANGLES, prim->mesh->indexBuffer->IndexCount(), GL_UNSIGNED_INT, 0);
			}
		}
    }

    void InitRendererGL(RenderDevice* device, Renderer** renderer)
    {
		RendererGL* rendererGL = NULL;
		*renderer = rendererGL = new RendererGL();
    }

    void ShutdownRendererGL(Renderer* renderer)
    {
        delete renderer;
    }
}