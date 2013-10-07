#include "renderer_gl.h"
#include "render_device_gl.h"
#include "camera.h"
#include "primitive.h"
#include "material.h"
#include "texture.h"
#include "mesh.h"
#include "scene.h"
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

		class RenderDevice* device;
		GLuint wireframeShader;
		GLuint diffuseTexShader;
		GLuint whiteTexture;
	};

    RendererGL::RendererGL() : wireframeShader(0)
    {

    }

    RendererGL::~RendererGL()
    {

    }

    void RendererGL::Render(const Camera* camera, const Scene* scene)
    {
		glClearColor(0.5, 0.5, 0.5, 1);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(diffuseTexShader);
		GLint positionAttributeLocation = glGetAttribLocation(diffuseTexShader, "position");
		GLint normalAttributeLocation = glGetAttribLocation(diffuseTexShader, "normal");
		GLint texAttributeLocation = glGetAttribLocation(diffuseTexShader, "texcoord");

		GLint modelMatLocation = glGetUniformLocation(diffuseTexShader, "modelMatrix");
		GLint viewMatLocation = glGetUniformLocation(diffuseTexShader, "viewMatrix");
		GLint projectionMatLocation = glGetUniformLocation(diffuseTexShader, "projectionMatrix");

		GLint texLocation = glGetUniformLocation(diffuseTexShader, "diffuseTex");


		Matrix4x4 viewMatrix = camera->ViewMatrix();
		Matrix4x4 projectionMatrix = camera->PerspectiveMatrix();
		glUniformMatrix4fv(viewMatLocation, 1, GL_TRUE, viewMatrix.FloatPtr());
		glUniformMatrix4fv(projectionMatLocation, 1, GL_TRUE, projectionMatrix.FloatPtr() );

		for(size_t i = 0; i < scene->primList.size(); i++)
		{
			const Primitive* prim = scene->primList[i].Get();
			
			glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, prim->ModelMatrix().FloatPtr());

			glBindBuffer(GL_ARRAY_BUFFER, prim->mesh->vertexBuffer->GetImpl()->vboID);
			glEnableVertexAttribArray(positionAttributeLocation);
			glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T2), 0);

			glEnableVertexAttribArray(normalAttributeLocation);
			glVertexAttribPointer(normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T2), (GLvoid*)(sizeof(float) * 3));
			
			size_t vertSize = sizeof(VertexP3N3T2);
			glEnableVertexAttribArray(texAttributeLocation);
			glVertexAttribPointer(texAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T2), (GLvoid*)(sizeof(float) * 6));
			

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->mesh->indexBuffer->GetImpl()->iboID);
			Matrix4x4 modelMatrix = prim->ModelMatrix();
			glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());

			if(prim->material->diffuseTex && prim->material->diffuseTex->hwTexture)
			{
				glUniform1i(texLocation, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, prim->material->diffuseTex->hwTexture->GetImpl()->id);
			}

			else
			{
				glUniform1i(texLocation, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D,whiteTexture);
			}
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLES, prim->mesh->indexBuffer->IndexCount(), GL_UNSIGNED_INT, 0);
		}
    }

    void InitRendererGL(RenderDevice* device, Renderer** renderer)
    {
		RendererGL* rendererGL = NULL;
		*renderer = rendererGL = new RendererGL();
        rendererGL->wireframeShader =  CreateProgram("shader/wireVertexShader.glsl", "shader/wirePixelShader.glsl");
		rendererGL->diffuseTexShader = CreateProgram("shader/diffuseTexVertexShader.glsl", "shader/diffuseTexPixelShader.glsl");

		rendererGL->whiteTexture = GenerateWhiteTexture();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_TEXTURE_2D);
    }

    void ShutdownRendererGL(Renderer* renderer)
    {
        delete renderer;
    }
}