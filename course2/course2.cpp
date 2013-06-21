#include <stdio.h>
#include "../window.h"
#include "../glUtility.h"
#include "../vector.h"
#include "../matrix.h"
#include "../Timer.h"

Window window;

struct Vertex
{
    Vector3 pos;
    Vector3 color;
};

GLuint vertexBuffer;
GLuint indexBuffer;
GLuint shaderProgram;

void InitResources()
{
    //load shader
    shaderProgram = CreateProgram("vertexShader.glsl", "pixelShader.glsl");

    struct Vertex triangleVertices[3];
    GLuint triangleIndices[3] = {0, 1, 2};

    //generate triangle vertex buffer
    triangleVertices[0].pos = Vector3(0, 1, 0);
    triangleVertices[0].color = Vector3(1, 0, 0);
    triangleVertices[1].pos = Vector3(-1, -1, 0);
    triangleVertices[1].color = Vector3(0, 1, 0);
    triangleVertices[2].pos = Vector3(1, -1, 0);
    triangleVertices[2].color = Vector3(0, 0, 1);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3, triangleVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3, triangleIndices, GL_STATIC_DRAW);

}

double frameTime = 0;
double animationTime = 0;
void Render()
{
	FastTimer timer;

	animationTime += (frameTime / 1000.f);

	timer.Start();

    glClearColor(0.5, 0.5, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(shaderProgram);
    GLint positionAttributeLocation = glGetAttribLocation(shaderProgram, "position");
    GLint colorAttributeLocation = glGetAttribLocation(shaderProgram, "color");

    GLint scaleMatLocation = glGetUniformLocation(shaderProgram, "scale");
    GLint offsetMatLocation = glGetUniformLocation(shaderProgram, "offset");

    Matrix4x4 scaleMatrix(0.5, 0, 0, 0,
                           0, 0.5, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1);
    Matrix4x4 offsetMatrix(1, 0, 0, 0,
                           0, 1, 0, 0.5 * sin(animationTime),
                           0, 0, 1, 0,
                           0, 0, 0, 1);

    glUniformMatrix4fv(scaleMatLocation, 1, GL_TRUE, (const GLfloat*)(&scaleMatrix));
    glUniformMatrix4fv(offsetMatLocation, 1, GL_TRUE, (const GLfloat*)(&offsetMatrix) );

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(positionAttributeLocation);
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    SwapBuffers(window.hdc);

	timer.End();

	frameTime = timer.GetDurationInMillisecnds();

}

void main()
{
	FastTimer::Initialize();

    if(!InitWindow(500, 500, window))
        return;
    
    if(!InitGL(window.hwnd, 0))
        return;

    InitResources();
    
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
            //if( GetMessage( &msg, NULL, 0, 0 ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        Render();
    }
}