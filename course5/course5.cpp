#include <stdio.h>
#include "../window.h"
#include "../gl_utility.h"
#include "../vector.h"
#include "../matrix.h"
#include "../Timer.h"
#include "../camera.h"
#include "../image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../stb_truetype.h"


Window window;

GLuint boxVertexBuffer;
GLuint boxIndexBuffer;

GLuint vertexLightingShader;
GLuint pixelLightingShader;

GLuint diffuseTexture;
GLuint whiteTexture;
GLuint diffuseSampler;

MyPerspectiveCamera camera;

double frameTime = 16;

float cameraMoveSpeed = 2.f;
float cameraTurnSpeed = 1.f;

enum RenderPath
{
    RP_VERTEX_LIGHT,
    RP_PIXEL_LIGHT,

    RP_VERTEX_LIGHT_TEX,
    RP_PIXEL_LIGHT_TEX,
};

RenderPath renderPath = RP_VERTEX_LIGHT;

struct BitmapChar
{
	wchar_t c;
	int width;
	int height;
	unsigned char* bitmap;
};

BitmapChar bye[23];
const wchar_t* byeStr = L"Goodbye IGS & CrazyPlug";

unsigned char ttf_buffer[(1<<25)];

class Light
{
public:
    Light()
    {
        dirPos = Vector4(1, 1, 1, 0);
        dirPos.Normalize();

        color = Vector3(1, 1, 1);
    }

    Vector4 dirPos;
    Vector3 color;
};

Light light;

class Material
{
public:
    Material()
    {
        diffuseAlbedo = Vector3(0.8, 0.8, 0.8);
        specularAlbedo = Vector3(0.8, 0, 0);
        specularExponent = 20.f;
    }

    Vector3 diffuseAlbedo;
    Vector3 specularAlbedo;
    float specularExponent;

};

Material material;

void InitResources()
{
    //load shader
    vertexLightingShader = CreateProgram("vertexLighting_vs.glsl", "vertexLighting_ps.glsl");

    pixelLightingShader = CreateProgram("pixelLighting_vs.glsl", "pixelLighting_ps.glsl");

    boxVertexBuffer = CreateSolidCubeVertexBuffer();
    boxIndexBuffer = CreateSolidCubeIndexBuffer();

    unsigned char* imgBuffer = NULL;
    int texWidth, texHeight;
    
	stbtt_fontinfo font;
	fread(ttf_buffer, 1, (1<<25), fopen("ARIALUNI.TTF", "rb"));
	stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));

	for(int i = 0; i < 23; i++)
	{
		bye[i].c = byeStr[i];
		int s = 32;
		bye[i].bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, s), bye[i].c, 
			&bye[i].width, &bye[i].height, 0,0);
	}

    if(LoadTGA("MazoIcon256.tga", &imgBuffer, &texWidth, &texHeight) )
    {
        //texture = CreateGLImage(imgBuffer, texWidth, texHeight);
        diffuseTexture = CreateMipmapGLImage(imgBuffer, texWidth, texHeight);
        FreeImageBuffer(imgBuffer);
    }

    whiteTexture = GenerateWhiteTexture();

    glGenSamplers(1, &diffuseSampler);
    glSamplerParameteri(diffuseSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(diffuseSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

        if(key == KEY_1)
        {
            renderPath = RP_VERTEX_LIGHT;
        }

        if(key == KEY_2)
        {
            renderPath = RP_PIXEL_LIGHT;
        }

        if(key == KEY_3)
        {
            renderPath = RP_VERTEX_LIGHT_TEX;
        }

        if(key == KEY_4)
        {
            renderPath = RP_PIXEL_LIGHT_TEX;
        }

        if(key == KEY_UP)
        {
            material.specularExponent += 5;
        }

        if(key == KEY_DOWN)
        {
            material.specularExponent -= 5;
            if(material.specularExponent <= 0)
                material.specularExponent = 0;
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
		camera.lookat.Normalize();
        camera.up = cross(camera.right, camera.lookat);
    }

    inputListener->ClearState();
}

void LightingPath(GLuint lightingShader)
{
    glUseProgram(lightingShader);

    GLint positionAttributeLocation = glGetAttribLocation(lightingShader, "position");
    GLint colorAttributeLocation = glGetAttribLocation(lightingShader, "color");
    GLint normalAttributeLocation = glGetAttribLocation(lightingShader, "normal");
    GLint texcoordAttributeLocation = glGetAttribLocation(lightingShader, "texcoord");

    glEnableVertexAttribArray(positionAttributeLocation);
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(SolidVertex), 0);
    glEnableVertexAttribArray(normalAttributeLocation);
    glVertexAttribPointer(normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(SolidVertex), (GLvoid*)(sizeof(float) * 3));
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(SolidVertex), (GLvoid*)(sizeof(float) * 6));
    glEnableVertexAttribArray(texcoordAttributeLocation);
    glVertexAttribPointer(texcoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(SolidVertex), (GLvoid*)(sizeof(float) * 9));

    //transform uniforms
    GLint modelMatLocation = glGetUniformLocation(lightingShader, "modelMatrix");
    GLint viewMatLocation = glGetUniformLocation(lightingShader, "viewMatrix");
    GLint projectionMatLocation = glGetUniformLocation(lightingShader, "projectionMatrix");

    //light uniforms
    GLint lightDirLocation = glGetUniformLocation(lightingShader, "lightPosDir");
    GLint lightColorLocation = glGetUniformLocation(lightingShader, "lightColor");
    
    //material uniforms
    GLint diffuseAlbedoLocation = glGetUniformLocation(lightingShader, "diffuseAlbedo");
    GLint specularAlbedoLocation = glGetUniformLocation(lightingShader, "specularAlbedo");
    GLint specularExponentLocation = glGetUniformLocation(lightingShader, "specularExponent");


    GLint diffuseTextLocation = glGetUniformLocation(lightingShader, "diffuseTexture");

    //setup light parameter
    glUniform4fv(lightDirLocation, 1, &light.dirPos[0]);
    glUniform3fv(lightColorLocation, 1, &light.color[0]);

    //setup material parameter
    glUniform3fv(diffuseAlbedoLocation, 1, &material.diffuseAlbedo[0]);
    glUniform3fv(specularAlbedoLocation, 1, &material.specularAlbedo[0]);
    glUniform1fv(specularExponentLocation, 1, &material.specularExponent);

    Matrix4x4 viewMatrix = camera.ViewMatrix();
    Matrix4x4 projectionMatrix = camera.Perspective();

    glUniformMatrix4fv(viewMatLocation, 1, GL_TRUE, viewMatrix.FloatPtr());
    glUniformMatrix4fv(projectionMatLocation, 1, GL_TRUE, projectionMatrix.FloatPtr() );
    
    glBindSampler(0, diffuseSampler);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(diffuseTextLocation, 0);

    if(renderPath == RP_VERTEX_LIGHT || renderPath == RP_PIXEL_LIGHT)
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
    else if(renderPath == RP_VERTEX_LIGHT_TEX || renderPath == RP_PIXEL_LIGHT_TEX)
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);

    glBindBuffer(GL_ARRAY_BUFFER, boxVertexBuffer);

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


	int rasterPos = -100;

	for(int i = 0; i < 23; i++)
	{
		int width = bye[i].width;
		int height = bye[i].height;

		if(bye[i].c == L' ')
		{
			rasterPos += 5;
		}

		for(int j = 0; j < height; j++)
		{
			for(int k = 0; k < width; k++)
			{
				unsigned char colorValue = bye[i].bitmap[(height - j - 1) * width + k];

				if(colorValue > 0)
				{
					float zOffset = 0.f;
					if(bye[i].c == 'y' || bye[i].c ==  'g') //hack to make it look right
						zOffset = -5.f;

					if(i > 13)
					{
						zOffset -= 30;
					}

					Matrix4x4 modelMatrix = Translate(Vector3( 15, (float)(rasterPos + k), (float)(j) + zOffset) ) * Scale(Vector3(0.5, 0.5, 0.5));
					glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, modelMatrix.FloatPtr());
					Vector3 cubeColor(1, 0, 0);
					glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
				}				
			}
		}
		rasterPos += (width + 2);

		if(i == 13) //hack to chage line
		{
			rasterPos = -60;
		}

	}

}

void Render(double frameTime)
{

    glClearColor(0.5, 0.5, 0.5, 1);
	glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(renderPath == RP_VERTEX_LIGHT || renderPath == RP_VERTEX_LIGHT_TEX)
       LightingPath(vertexLightingShader);

    else if(renderPath == RP_PIXEL_LIGHT || renderPath == RP_PIXEL_LIGHT_TEX)
       LightingPath(pixelLightingShader);

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


    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
		timer.Start();
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
            //if( GetMessage( &msg, NULL, 0, 0 ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        InputControl(frameTime);
        Render(frameTime);

		timer.End();
		frameTime = timer.GetDurationInMillisecnds();
    }

}