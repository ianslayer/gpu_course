#include "renderer_gl.h"
#include "render_device_gl.h"
#include "camera.h"
#include "primitive.h"
#include "mesh.h"
#include "../matrix.h"

namespace jade
{
    RendererGL::RendererGL() : wireframeShader(0)
    {

    }

    RendererGL::~RendererGL()
    {

    }

    void RendererGL::Render(const Camera* camera, const Scene* scene)
    {

    }

    void RendererGL::RenderWireframe(const Camera* camera, const class Primitive* prim)
    {
        glUseProgram(wireframeShader);
        GLint positionAttributeLocation = glGetAttribLocation(wireframeShader, "position");

        GLint modelMatLocation = glGetUniformLocation(wireframeShader, "modelMatrix");
        GLint viewMatLocation = glGetUniformLocation(wireframeShader, "viewMatrix");
        GLint projectionMatLocation = glGetUniformLocation(wireframeShader, "projectionMatrix");

        Matrix4x4 viewMatrix = camera->ViewMatrix();
        Matrix4x4 projectionMatrix = camera->PerspectiveMatrix();
        glUniformMatrix4fv(viewMatLocation, 1, GL_TRUE, viewMatrix.FloatPtr());
        glUniformMatrix4fv(projectionMatLocation, 1, GL_TRUE, projectionMatrix.FloatPtr() );

        glBindBuffer(GL_ARRAY_BUFFER, prim->mesh->vertexBuffer->GetImpl()->vboID);
        glEnableVertexAttribArray(positionAttributeLocation);
        glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexP3N3T2), 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->mesh->indexBuffer->GetImpl()->iboID);
        Matrix4x4 modelMatrix = prim->ModelMatrix();
        glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, prim->mesh->indexBuffer->IndexCount(), GL_UNSIGNED_INT, 0);
    }

    void InitRendererGL(RenderDevice* device, RendererGL** renderer)
    {
        *renderer = new RendererGL();
        (*renderer)->wireframeShader =  CreateProgram("shader/wireVertexShader.glsl", "shader/wirePixelShader.glsl");

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_TEXTURE_2D);
    }

    void ShutdownRendererGL(RendererGL* renderer)
    {
        delete renderer;
    }
}