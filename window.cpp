#include "window.h"

const TCHAR* windowClass = TEXT("jog2MainWindow");
const TCHAR* windowTitle = TEXT("jog2RenderWindow");

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool InitWindow(int width, int height, Window& window)
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

    AdjustWindowRect( &rc, windowStyle, FALSE );
    MoveWindow(window.hwnd, 0, 0, rc.right - rc.left, rc.bottom - rc.top, TRUE);

    window.hdc = GetDC(window.hwnd);

    return true;
}