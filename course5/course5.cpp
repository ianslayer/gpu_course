#include <stdio.h>
#include "../window.h"
#include "../glUtility.h"
#include "../vector.h"
#include "../matrix.h"
#include "../Timer.h"
#include "../camera.h"
#include "../image.h"

Window window;

GLuint boxVertexBuffer;
GLuint boxIndexBuffer;
GLuint shaderProgram;

GLuint texture;

MyPerspectiveCamera camera;

double frameTime = 16;

float cameraMoveSpeed = 2.f;
float cameraTurnSpeed = 1.f;

void InitResources()
{
    //load shader
    shaderProgram = CreateProgram("vertexShader.glsl", "pixelShader.glsl");

    boxVertexBuffer = CreateSolidCubeVertexBuffer();
    boxIndexBuffer = CreateSolidCubeIndexBuffer();

    const unsigned char* imgBuffer = NULL;
    int texWidth, texHeight;
    
    if(LoadTGA("MazoIcon256.tga", &imgBuffer, &texWidth, &texHeight) )
    {
        //texture = CreateGLImage(imgBuffer, texWidth, texHeight);
        texture = CreateMipmapGLImage(imgBuffer, texWidth, texHeight);
        FreeImageBuffer(imgBuffer);
    }
}

void InitGLState()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_TEXTURE_2D);
}

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

class Light
{
public:
	Light()
	{
		dirPos = Vector4(1, 1, 1, 0);
		dirPos.Normalize();
	}

	Vector4 dirPos;
};

Light light;

void Render(double frameTime)
{

    glClearColor(0.5, 0.5, 0.5, 1);
	glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);
    GLint positionAttributeLocation = glGetAttribLocation(shaderProgram, "position");
    GLint colorAttributeLocation = glGetAttribLocation(shaderProgram, "color");
    GLint normalAttributeLocation = glGetAttribLocation(shaderProgram, "normal");
	GLint texcoordAttributeLocation = glGetAttribLocation(shaderProgram, "texcoord");

    GLint modelMatLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
    GLint viewMatLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
    GLint projectionMatLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
    GLint cubeColorLocation = glGetUniformLocation(shaderProgram, "cubeColor");
	GLint lightDirLocation = glGetUniformLocation(shaderProgram, "lightPosDir");
    GLint diffuseTextLocation = glGetUniformLocation(shaderProgram, "diffuseTexture");

    Matrix4x4 viewMatrix = camera.ViewMatrix();
    Matrix4x4 projectionMatrix = camera.Perspective();
    Vector3 cubeColor(1, 1, 1);

    glUniformMatrix4fv(viewMatLocation, 1, GL_TRUE, viewMatrix.FloatPtr());
    glUniformMatrix4fv(projectionMatLocation, 1, GL_TRUE, projectionMatrix.FloatPtr() );
    glUniform3fv(cubeColorLocation, 1, &cubeColor[0]);
	glUniform4fv(lightDirLocation, 1, &light.dirPos[0]);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(diffuseTextLocation, 0);
    glBindTexture(GL_TEXTURE_2D, texture);


    glBindBuffer(GL_ARRAY_BUFFER, boxVertexBuffer);
    glEnableVertexAttribArray(positionAttributeLocation);
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(SolidVertex), 0);
	glEnableVertexAttribArray(normalAttributeLocation);
	glVertexAttribPointer(normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(SolidVertex), (GLvoid*)(sizeof(float) * 3));
    //glEnableVertexAttribArray(colorAttributeLocation);
    //glVertexAttribPointer(colorAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(SolidVertex), (GLvoid*)(sizeof(float) * 6));
	glEnableVertexAttribArray(texcoordAttributeLocation);
	glVertexAttribPointer(texcoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(SolidVertex), (GLvoid*)(sizeof(float) * 9));
	

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxIndexBuffer);

    Matrix4x4 modelMatrix;

    //draw first box
    modelMatrix = Translate(Vector3(-4, -1.8, 0) ) * Scale(Vector3(1, 1, 1));
    glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //draw second box
    modelMatrix = Translate(Vector3(-5, 1.8, 0) ) * Scale(Vector3(1.5, 1.5, 1.5));
    glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

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
    InitGLState();

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