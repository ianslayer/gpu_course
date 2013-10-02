#include "../window.h"
#include "../gl_utility.h"
#include <stdio.h>

Window window;

struct Vertex
{
    float x, y, z;
    float r, g, b;
};

GLuint vertexBuffer;
GLuint indexBuffer;

GLuint vertexArray;

GLuint shaderProgram;

void InitResources()
{
    //load shader
    shaderProgram = CreateProgram("vertexShader.glsl", "pixelShader.glsl");

    struct Vertex triangleVertices[3];
    GLuint triangleIndices[3] = {0, 1, 2};

    //generate triangle vertex buffer
    triangleVertices[0].x = 0, triangleVertices[0].y = 1, triangleVertices[0].z = 0; 
    triangleVertices[0].r = 1, triangleVertices[0].g = 0, triangleVertices[0].b = 0; 
    triangleVertices[1].x = -1, triangleVertices[1].y = -1, triangleVertices[1].z = 0;
    triangleVertices[1].r = 0, triangleVertices[1].g = 1, triangleVertices[1].b = 0; 
    triangleVertices[2].x = 1, triangleVertices[2].y = -1, triangleVertices[2].z = 0; 
    triangleVertices[2].r = 0, triangleVertices[2].g = 0, triangleVertices[2].b = 1; 

    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3, triangleVertices, GL_STATIC_DRAW);

    GLint positionAttributeLocation = glGetAttribLocation(shaderProgram, "position");
    GLint colorAttributeLocation = glGetAttribLocation(shaderProgram, "color");

    glEnableVertexAttribArray(positionAttributeLocation);
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));


    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3, triangleIndices, GL_STATIC_DRAW);

}

void Render()
{
    glClearColor(0.5, 0.5, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
	glViewport(0, 0, window.width, window.height);

    glUseProgram(shaderProgram);
    GLint positionAttributeLocation = glGetAttribLocation(shaderProgram, "position");
    GLint colorAttributeLocation = glGetAttribLocation(shaderProgram, "color");

    glBindVertexArray(vertexArray);
    /*
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(positionAttributeLocation);
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));
    */

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    //glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    SwapBuffers(window.hdc);
}

void main()
{
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