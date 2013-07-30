#include <stdio.h>
#include "../window.h"
#include "../glUtility.h"
#include "../vector.h"
#include "../matrix.h"
#include "../Timer.h"
#include "../camera.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../stb_truetype.h"

Window window;

GLuint boxVertexBuffer;
GLuint boxIndexBuffer;
GLuint shaderProgram;
MyPerspectiveCamera camera;

double frameTime = 16;

float cameraMoveSpeed = 2.f;
float cameraTurnSpeed = 1.f;

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

    virtual void OnMouseButton(const int x, const int y, const MouseButton button, const bool pressed)
    {
        if(pressed && button == MOUSE_RIGHT)
        {
            mouseControl = true;
        }
        else
        {
            mouseControl = false;
        }
    }
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

    }

    void ClearState()
    {
        rx = 0;
        ry = 0;
    }

    bool forward;
    bool backward;
    bool right;
    bool left;

    bool mouseControl;
    float rx;
    float ry;
};

MyInputListener* inputListener = NULL;

struct BitmapChar
{
	wchar_t c;
	int width;
	int height;
	unsigned char* bitmap;
};

BitmapChar attackOnTitan[16];
BitmapChar attackOnTitanCh[5];
const wchar_t* attackOnTitanStr = L"Attack on Titan!";
const wchar_t* AttackOnTitanChStr = L"¶iÀ»ªº¥¨¤H";

unsigned char ttf_buffer[(1<<25)];
void InitResources()
{
	//load shader
	shaderProgram = CreateProgram("vertexShader.glsl", "pixelShader.glsl");

	boxVertexBuffer = CreateCubeVertexBuffer();
	boxIndexBuffer = CreateWireCubeIndexBuffer();

	stbtt_fontinfo font;
    fread(ttf_buffer, 1, (1<<25), fopen("ARIALUNI.TTF", "rb"));
	stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));

	//attack on titan !!!
	for(int i = 0; i < 16; i++)
	{
		attackOnTitan[i].c = attackOnTitanStr[i];
		int s = 32;
		attackOnTitan[i].bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, s), attackOnTitan[i].c, 
			&attackOnTitan[i].width, &attackOnTitan[i].height, 0,0);
	}

    for(int i = 0; i < 5; i++)
    {
        attackOnTitanCh[i].c = AttackOnTitanChStr[i];
        int s = 32;
        attackOnTitanCh[i].bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, s), attackOnTitanCh[i].c, 
            &attackOnTitanCh[i].width, &attackOnTitanCh[i].height, 0,0);
    }
}

void InputControl(double frameTime)
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
        camera.up = cross(camera.right, camera.lookat);
    }

    inputListener->ClearState();
}

void RenderAttackOnTitan()
{
	int rasterPos = -100;

	GLint modelMatLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
	GLint cubeColorLocation = glGetUniformLocation(shaderProgram, "cubeColor");

	for(int i = 0; i < 16; i++)
	{
		int width = attackOnTitan[i].width;
		int height = attackOnTitan[i].height;

		if(attackOnTitan[i].c == L' ')
		{
			rasterPos += 5;
		}

		for(int j = 0; j < height; j++)
		{
			for(int k = 0; k < width; k++)
			{
				unsigned char colorValue = attackOnTitan[i].bitmap[(height - j - 1) * width + k];

				if(colorValue > 0)
				{
					Matrix4x4 modelMatrix = Translate(Vector3( 15, (float)(rasterPos + k), (float)(j)) ) * Scale(Vector3(0.5, 0.5, 0.5));
					glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());
					Vector3 cubeColor(1, 0, 0);
					glUniform3fv(cubeColorLocation, 1, &cubeColor[0]);
					glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
				}				
			}
		}
		rasterPos += (width + 2);
	}

	rasterPos = -60;
    for(int i = 0; i < 5; i++)
    {
        int width = attackOnTitanCh[i].width;
        int height = attackOnTitanCh[i].height;

        if(attackOnTitanCh[i].c == L' ')
        {
            rasterPos += 5;
        }

        for(int j = 0; j < height; j++)
        {
            for(int k = 0; k < width; k++)
            {
                unsigned char colorValue = attackOnTitanCh[i].bitmap[(height - j - 1) * width + k];

                if(colorValue > 0)
                {
                    Matrix4x4 modelMatrix = Translate(Vector3( 10, (float)(rasterPos + k), (float)(j) + 15) ) * Scale(Vector3(0.5, 0.5, 0.5));
                    glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());
                    Vector3 cubeColor(1, 0, 0);
                    glUniform3fv(cubeColorLocation, 1, &cubeColor[0]);
                    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
                }				

            }
        }
        rasterPos += (width + 2);
    }
}

void Render(double frameTime)
{

	glClearColor(0.5, 0.5, 0.5, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	GLint positionAttributeLocation = glGetAttribLocation(shaderProgram, "position");
	GLint colorAttributeLocation = glGetAttribLocation(shaderProgram, "color");

	GLint modelMatLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
	GLint viewMatLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	GLint projectionMatLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	GLint cubeColorLocation = glGetUniformLocation(shaderProgram, "cubeColor");

    Matrix4x4 viewMatrix = camera.ViewMatrix();
    Matrix4x4 projectionMatrix = camera.SimplePerspective();
	Vector3 cubeColor(1, 1, 1);

	glUniformMatrix4fv(viewMatLocation, 1, GL_TRUE, viewMatrix.FloatPtr());
	glUniformMatrix4fv(projectionMatLocation, 1, GL_TRUE, projectionMatrix.FloatPtr() );
	glUniform3fv(cubeColorLocation, 1, &cubeColor[0]);

	glBindBuffer(GL_ARRAY_BUFFER, boxVertexBuffer);
	glEnableVertexAttribArray(positionAttributeLocation);
	glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
	glEnableVertexAttribArray(colorAttributeLocation);
	glVertexAttribPointer(colorAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxIndexBuffer);

	//draw first box
	Matrix4x4 modelMatrix = Translate(Vector3(-4, -1.8, 0) ) * Scale(Vector3(1, 1, 1));
	glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

	//draw second box
	modelMatrix = Translate(Vector3(-5, 1.8, 0) ) * Scale(Vector3(1.5, 1.5, 1.5));
	glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

	//attack on titan!!!
	RenderAttackOnTitan();

	SwapBuffers(window.hdc);

}

void main()
{
    FastTimer::Initialize();

	if(!InitWindow(800, 800, window))
		return;

    inputListener = new MyInputListener();
    SetInputListener(inputListener);

	if(!InitGL(window.hwnd, 4))
		return;

	InitResources();

    FastTimer timer;

    timer.Start();
	MSG msg = {0};
	while( WM_QUIT != msg.message )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			//if( GetMessage( &msg, NULL, 0, 0 ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
        InputControl(frameTime);
        Render(frameTime);
	}
    timer.End();
    frameTime = timer.GetDurationInMillisecnds();
}