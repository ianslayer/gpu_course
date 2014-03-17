#ifndef WINDOW_H
#define WINDOW_H

#define NOMINMAX

#ifdef _WIN32
    #include <windows.h>
#endif

struct Window
{
#ifdef _WIN32
    HWND hwnd;
    HDC hdc;
#else if defined (__APPLE__)
    NSWindow* window;
#endif    
    int width;
    int height;
};

enum MouseButton
{
    MOUSE_LEFT   = 0,
    MOUSE_MIDDLE = 1,
    MOUSE_RIGHT  = 2,
};

class InputListener
{
public:
    virtual void OnMouseMove(const int x, const int y, const int deltaX, const int deltaY) {};
    virtual void OnMouseButton(const int x, const int y, const MouseButton button, const bool pressed) {};
    virtual void OnMouseWheel(const int x, const int y, const int scroll) {};
    virtual void OnKey(const unsigned int key, const bool pressed) {};
};

bool InitWindow(int width, int height, Window& window, InputListener* inputListener = nullptr);
void SetInputListener(InputListener* listener);
void CaptureMouse(Window& window, const bool value);

#ifdef _WIN32
// Platform independent key codes
#define KEY_0 int('0')
#define KEY_1 int('1')
#define KEY_2 int('2')
#define KEY_3 int('3')
#define KEY_4 int('4')
#define KEY_5 int('5')
#define KEY_6 int('6')
#define KEY_7 int('7')
#define KEY_8 int('8')
#define KEY_9 int('9')

#define KEY_LEFT      VK_LEFT
#define KEY_RIGHT     VK_RIGHT
#define KEY_UP        VK_UP
#define KEY_DOWN      VK_DOWN
#define KEY_CTRL      VK_CONTROL
#define KEY_SHIFT     VK_SHIFT
#define KEY_ENTER     VK_RETURN
#define KEY_SPACE     VK_SPACE
#define KEY_TAB       VK_TAB
#define KEY_ESCAPE    VK_ESCAPE
#define KEY_BACKSPACE VK_BACK
#define KEY_HOME      VK_HOME
#define KEY_END       VK_END
#define KEY_INSERT    VK_INSERT
#define KEY_DELETE    VK_DELETE

#define KEY_F1  VK_F1
#define KEY_F2  VK_F2
#define KEY_F3  VK_F3
#define KEY_F4  VK_F4
#define KEY_F5  VK_F5
#define KEY_F6  VK_F6
#define KEY_F7  VK_F7
#define KEY_F8  VK_F8
#define KEY_F9  VK_F9
#define KEY_F10 VK_F10
#define KEY_F11 VK_F11
#define KEY_F12 VK_F12

#define KEY_NUMPAD0 VK_NUMPAD0
#define KEY_NUMPAD1 VK_NUMPAD1
#define KEY_NUMPAD2 VK_NUMPAD2
#define KEY_NUMPAD3 VK_NUMPAD3
#define KEY_NUMPAD4 VK_NUMPAD4
#define KEY_NUMPAD5 VK_NUMPAD5
#define KEY_NUMPAD6 VK_NUMPAD6
#define KEY_NUMPAD7 VK_NUMPAD7
#define KEY_NUMPAD8 VK_NUMPAD8
#define KEY_NUMPAD9 VK_NUMPAD9

#define KEY_ADD        VK_ADD
#define KEY_SUBTRACT   VK_SUBTRACT
#define KEY_MULTIPLY   VK_MULTIPLY
#define KEY_DIVIDE     VK_DIVIDE
#define KEY_SEPARATOR  VK_SEPARATOR
#define KEY_DECIMAL    VK_DECIMAL
#define KEY_PAUSE      VK_PAUSE

#define KEY_A int('A')
#define KEY_B int('B')
#define KEY_C int('C')
#define KEY_D int('D')
#define KEY_E int('E')
#define KEY_F int('F')
#define KEY_G int('G')
#define KEY_H int('H')
#define KEY_I int('I')
#define KEY_J int('J')
#define KEY_K int('K')
#define KEY_L int('L')
#define KEY_M int('M')
#define KEY_N int('N')
#define KEY_O int('O')
#define KEY_P int('P')
#define KEY_Q int('Q')
#define KEY_R int('R')
#define KEY_S int('S')
#define KEY_T int('T')
#define KEY_U int('U')
#define KEY_V int('V')
#define KEY_W int('W')
#define KEY_X int('X')
#define KEY_Y int('Y')
#define KEY_Z int('Z')

#elif defined(__APPLE__)

#define KEY_0 int('0')
#define KEY_1 int('1')
#define KEY_2 int('2')
#define KEY_3 int('3')
#define KEY_4 int('4')
#define KEY_5 int('5')
#define KEY_6 int('6')
#define KEY_7 int('7')
#define KEY_8 int('8')
#define KEY_9 int('9')

#define KEY_LEFT
#define KEY_RIGHT
#define KEY_UP
#define KEY_DOWN
#define KEY_CTRL
#define KEY_SHIFT
#define KEY_ENTER
#define KEY_SPACE
#define KEY_TAB
#define KEY_ESCAPE
#define KEY_BACKSPACE
#define KEY_HOME
#define KEY_END
#define KEY_INSERT
#define KEY_DELETE

#define KEY_F1
#define KEY_F2
#define KEY_F3
#define KEY_F4
#define KEY_F5
#define KEY_F6
#define KEY_F7
#define KEY_F8
#define KEY_F9
#define KEY_F10
#define KEY_F11
#define KEY_F12

#define KEY_NUMPAD0
#define KEY_NUMPAD1
#define KEY_NUMPAD2
#define KEY_NUMPAD3
#define KEY_NUMPAD4
#define KEY_NUMPAD5
#define KEY_NUMPAD6
#define KEY_NUMPAD7
#define KEY_NUMPAD8
#define KEY_NUMPAD9

#define KEY_ADD
#define KEY_SUBTRACT
#define KEY_MULTIPLY
#define KEY_DIVIDE
#define KEY_SEPARATOR
#define KEY_DECIMAL
#define KEY_PAUSE

#define KEY_A int('a')
#define KEY_B int('b')
#define KEY_C int('c')
#define KEY_D int('d')
#define KEY_E int('e')
#define KEY_F int('f')
#define KEY_G int('g')
#define KEY_H int('h')
#define KEY_I int('i')
#define KEY_J int('j')
#define KEY_K int('k')
#define KEY_L int('l')
#define KEY_M int('m')
#define KEY_N int('n')
#define KEY_O int('o')
#define KEY_P int('p')
#define KEY_Q int('q')
#define KEY_R int('r')
#define KEY_S int('s')
#define KEY_T int('t')
#define KEY_U int('u')
#define KEY_V int('v')
#define KEY_W int('w')
#define KEY_X int('x')
#define KEY_Y int('y')
#define KEY_Z int('z')

#endif

#endif