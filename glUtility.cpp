#include <fstream>
#include "glUtility.h"

#pragma comment(lib,"opengl32.lib")
using namespace std;

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB; 
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

extern "C"
{
    void open_libgl(void);
    void close_libgl(void);
    void *get_proc(const char *proc);
};

void GetWGLExt()
{
    open_libgl();
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) get_proc("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) get_proc("wglCreateContextAttribsARB");
    close_libgl();
}

LRESULT CALLBACK FakeWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...
        EndPaint(hWnd, &ps);
        break;
    //case WM_DESTROY:
       // PostQuitMessage(0);
       // break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool InitGL(HWND hwnd, int msaaCount)
{

    //create fake window
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);

    DWORD windowStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    wcex.style			= CS_HREDRAW | CS_VREDRAW ;
    wcex.lpfnWndProc	= FakeWndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= NULL;
    wcex.hIcon			= NULL;
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= TEXT("fakeMainWindow");
    wcex.hIconSm		= NULL;

    RegisterClassEx(&wcex);

    HWND fakeWinHwnd = CreateWindowEx(0, TEXT("fakeMainWindow"),  TEXT("fakeWindow"),
        windowStyle, 0, 0, 512, 512, NULL, NULL, NULL, NULL);

    HDC fakeDc;
    fakeDc = GetDC(fakeWinHwnd);
    int result; 
    PIXELFORMATDESCRIPTOR  pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0,0,0,0,0,0,0,0,0,0,0,0,0, // useless parameters
        32,
        0,0,0,0,0,0,0
    };

    int indexPixelFormat = ChoosePixelFormat(fakeDc, &pfd);
    result = SetPixelFormat(fakeDc, indexPixelFormat, &pfd);

    HGLRC fakeHglrc = wglCreateContext(fakeDc);

    wglMakeCurrent(fakeDc, fakeHglrc);
    GetWGLExt();

    int enableMSAA = GL_FALSE;
    if(msaaCount > 1)
        enableMSAA = GL_TRUE;

    //create real context
    int attributeListInt[] = {
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         24,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        WGL_ACCUM_BITS_ARB,         0,
        WGL_SAMPLE_BUFFERS_ARB,     enableMSAA, // MSAA on
        WGL_SAMPLES_ARB,            msaaCount, 
        0, 0
    };

     float fAttributes[] = {0,0};

    int pixelFormat[1];
    unsigned int formatCount;

    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    int attributeList[5];

    HDC hdc = GetDC(hwnd);
    // Query for a pixel format that fits the attributes we want.
    result = wglChoosePixelFormatARB(hdc, attributeListInt, fAttributes, 1, pixelFormat, &formatCount);
    if(result != 1)
    {
        return false;
    }
    result = DescribePixelFormat(hdc, pixelFormat[0], sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDescriptor);
    // If the video card/display can handle our desired pixel format then we set it as the current one.
    result = SetPixelFormat(hdc, pixelFormat[0], &pixelFormatDescriptor);
    DWORD error = 0;
     if(result != 1)
     {
         error = GetLastError();
         return false;
     }
   // int attriblist[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3, 
    //                    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
    //                    0, 0};

    HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, 0);
 
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(fakeHglrc);
    ReleaseDC(fakeWinHwnd, fakeDc);
    DestroyWindow(fakeWinHwnd);
    UnregisterClass(TEXT("fakeMainWindow"), 0);

    BOOL success = wglMakeCurrent(hdc, hglrc);

    if(!success)
        return false;

    int initGL = gl3wInit();
    if(initGL != 0)
        return false;

    GetWGLExt();

    ReleaseDC(hwnd, hdc);
    

    return true;
}

unsigned long GetFileLength(ifstream& file)
{
    if(!file.good()) return 0;

    unsigned long pos=file.tellg();
    file.seekg(0,ios::end);
    unsigned long len = file.tellg();
    file.seekg(ios::beg);

    return len;
}

int LoadShader(char* filename, GLchar** ShaderSource, GLint* len)
{
    ifstream file;
    file.open(filename, ios::in); // opens as ASCII!
    if(!file) return -1;

    *len = GetFileLength(file);

    if (len==0) return -2;   // Error: Empty File 

    *ShaderSource = (GLchar*) new GLchar[(*len)+1];
    if (*ShaderSource == 0) return -3;   // can't reserve memory

    // len isn't always strlen cause some characters are stripped in ascii read...
    // it is important to 0-terminate the real length later, len is just max possible value... 
    (*ShaderSource)[(*len)] = (GLchar) 0; 

    unsigned int i=0;
    while (file.good())
    {
        (*ShaderSource)[i] = file.get();       // get character from file.
        if (!file.eof())
            i++;
    }

    (*ShaderSource)[i] = 0;  // 0-terminate it at the correct position

    file.close();

    return 0; // No Error
}


int UnloadShader(GLchar** ShaderSource)
{
    if (*ShaderSource != 0)
        delete[] *ShaderSource;
    *ShaderSource = 0;

    return 0;
}

GLuint CompileShader(char* filename, GLuint shaderType)
{
    GLchar* shaderSource = 0;
    GLint shaderLen = 0;
    LoadShader(filename, &shaderSource, &shaderLen);
    GLuint shader = glCreateShader(shaderType);

    if(shaderLen == 0)
        return 0;

    glShaderSource(shader, 1, &shaderSource, &shaderLen);

    glCompileShader(shader);

    GLint compiled;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_FALSE)
    {
        printf("compile shader: %s success\n", filename);
    }     
    else
    {
        printf("compile shader: %s failed\n", filename);

        int infologLength = 0;

        int charsWritten  = 0;
        char *infoLog;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH,&infologLength);
        if (infologLength > 0)
        {
            infoLog = new char[infologLength];
            glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);

            printf(infoLog);
            delete [] infoLog;
        }

    }

    UnloadShader(&shaderSource);

    return shader;
}

GLuint CreateProgram(char* vsFilename, char* psFileName)
{
   //load shader
    GLuint vertexShader = CompileShader(vsFilename, GL_VERTEX_SHADER);
    GLuint pixelShader = CompileShader(psFileName, GL_FRAGMENT_SHADER);
    GLuint shaderProgram = glCreateProgram();


    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, pixelShader);

    glLinkProgram(shaderProgram);

    GLint linked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (linked)
    {
        // ok!
    } 
    else
    {
        printf("program link error\n");
    }
    return shaderProgram;
}

GLuint CreateCubeVertexBuffer()
{
	GLuint cubeVertexBuffer;

	MyVertex cube[8];
	cube[0].position = Vector3(1, 1, 1);
	cube[1].position = Vector3(1, 1, -1);
	cube[2].position = Vector3(1, -1, 1);
	cube[3].position = Vector3(1, -1, -1);
	cube[4].position = Vector3(-1, 1, 1);
	cube[5].position = Vector3(-1, 1, -1);
	cube[6].position = Vector3(-1, -1, 1);
	cube[7].position = Vector3(-1, -1, -1);

	for(int i = 0; i < 8; i++)
	{
		cube[i].color = Vector3(1, 1, 1);
	}

	glGenBuffers(1, &cubeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

	return cubeVertexBuffer;

}

GLuint CreateWireCubeIndexBuffer()
{
	GLuint wireCubeIndexBuffer;

	GLuint wireCubeIndices[24] = {0, 2,
		1, 3,
		5, 7,
		4, 6,
		0, 4,
		1, 5,
		3, 7,
		2, 6,
		0, 1,
		4, 5,
		6, 7,
		2, 3};


	glGenBuffers(1, &wireCubeIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wireCubeIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wireCubeIndices), wireCubeIndices, GL_STATIC_DRAW);

	return wireCubeIndexBuffer;
}