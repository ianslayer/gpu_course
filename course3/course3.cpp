#include <stdio.h>
#include "../window.h"
#include "../glUtility.h"
#include "../vector.h"
#include "../matrix.h"
#include "../Timer.h"
#include "../camera.h"

Window window;

GLuint boxVertexBuffer;
GLuint boxIndexBuffer;
GLuint shaderProgram;
MyPerspectiveCamera camera;

void InitResources()
{
	//load shader
	shaderProgram = CreateProgram("vertexShader.glsl", "pixelShader.glsl");

	boxVertexBuffer = CreateCubeVertexBuffer();
	boxIndexBuffer = CreateWireCubeIndexBuffer();

}

void Render()
{

	glClearColor(0.5, 0.5, 0.5, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	GLint positionAttributeLocation = glGetAttribLocation(shaderProgram, "position");
	GLint colorAttributeLocation = glGetAttribLocation(shaderProgram, "color");

	GLint modelMatLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
	GLint viewMatLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	GLint projectionMatLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");

    Matrix4x4 viewMatrix = camera.ViewMatrix();
    Matrix4x4 projectionMatrix = camera.SimplePerspective();

	glUniformMatrix4fv(viewMatLocation, 1, GL_TRUE, viewMatrix.FloatPtr());
	glUniformMatrix4fv(projectionMatLocation, 1, GL_TRUE, projectionMatrix.FloatPtr() );

	glBindBuffer(GL_ARRAY_BUFFER, boxVertexBuffer);
	glEnableVertexAttribArray(positionAttributeLocation);
	glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
	glEnableVertexAttribArray(colorAttributeLocation);
	glVertexAttribPointer(colorAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxIndexBuffer);

	//draw first box
	Matrix4x4 modelMatrix = Translate(Vector3(-4, -2, 0) ) * Scale(Vector3(1, 1, 1));
	glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

	//draw second box
	modelMatrix = Translate(Vector3(-5, 2, 0) ) * Scale(Vector3(1.5, 1.5, 1.5));
	glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

	SwapBuffers(window.hdc);

}

void main()
{
	if(!InitWindow(800, 800, window))
		return;

	if(!InitGL(window.hwnd, 4))
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