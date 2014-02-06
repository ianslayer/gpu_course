#include "window.h"


#ifdef _WIN32
const TCHAR* windowClass = TEXT("jog2MainWindow");
const TCHAR* windowTitle = TEXT("jog2RenderWindow");

#define GETX(l) (int(l & 0xFFFF))
#define GETY(l) (int(l) >> 16)

InputListener* gInputListener = NULL;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...
        EndPaint(hWnd, &ps);
        break;
    case WM_MOUSEMOVE:
        static int lastX, lastY;
        int x, y;
        x = GETX(lParam);
        y = GETY(lParam);
        if(gInputListener)
            gInputListener->OnMouseMove(x, y, x - lastX, y - lastY);
        lastX = x;
        lastY = y;
        break;
    case WM_KEYDOWN:
        if(gInputListener)
            gInputListener->OnKey((unsigned int) wParam, true);
        break;
    case WM_KEYUP:
        if(gInputListener)
            gInputListener->OnKey((unsigned int) wParam, false);
        break;
    case WM_LBUTTONDOWN:
        if(gInputListener)
            gInputListener->OnMouseButton(GETX(lParam), GETY(lParam), MOUSE_LEFT, true);
        break;
    case WM_LBUTTONUP:
        if(gInputListener)
            gInputListener->OnMouseButton(GETX(lParam), GETY(lParam), MOUSE_LEFT, false);
        break;
    case WM_RBUTTONDOWN:
        if(gInputListener)
            gInputListener->OnMouseButton(GETX(lParam), GETY(lParam), MOUSE_RIGHT, true);
        break;
    case WM_RBUTTONUP:
        if(gInputListener)
            gInputListener->OnMouseButton(GETX(lParam), GETY(lParam), MOUSE_RIGHT, false);
        break;
    case WM_MOUSEWHEEL:
        static int scroll;
        int s;

        scroll += GET_WHEEL_DELTA_WPARAM(wParam);
        s = scroll / WHEEL_DELTA;
        scroll %= WHEEL_DELTA;

        POINT point;
        point.x = GETX(lParam);
        point.y = GETY(lParam);
        ScreenToClient(hWnd, &point);

        if (s != 0 && gInputListener) gInputListener->OnMouseWheel(point.x, point.y, s);
        break;   

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool InitWindow(int width, int height, Window& window, InputListener* inputListener)
{
    window.width = width;
    window.height = height;

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);

    DWORD windowStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    wcex.style			= CS_HREDRAW | CS_VREDRAW ;
    wcex.lpfnWndProc	= WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= NULL;
    wcex.hIcon			= NULL;
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= windowClass;
    wcex.hIconSm		= NULL;

    RegisterClassEx(&wcex);

    window.hwnd = CreateWindowEx(0, windowClass, windowTitle,
        windowStyle, 0, 0, window.width, window.height, NULL, NULL, NULL, NULL);

    if(!window.hwnd)
    {
        return false;
    }

    ShowWindow(window.hwnd, SW_SHOW);

    RECT rc = { 0, 0, window.width, window.height };

    AdjustWindowRect( &rc, windowStyle, FALSE);
    MoveWindow(window.hwnd, 0, 0, rc.right - rc.left, rc.bottom - rc.top, TRUE);

    window.hdc = GetDC(window.hwnd);

    return true;
}

void SetInputListener(InputListener* listener)
{
    gInputListener = listener;
}

void SetCursorPos(HWND hWnd, const int x, const int y){
    POINT point = { x, y };
    ClientToScreen(hWnd, &point);
    SetCursorPos(point.x, point.y);
}

void CaptureMouse(Window& window, const bool value)
{
    static POINT point;
    if (value){
        GetCursorPos(&point);
        SetCursorPos(window.hwnd, window.width / 2, window.height / 2);
    } else {
        SetCursorPos(point.x, point.y);
    }
    ShowCursor(value);
}

#endif