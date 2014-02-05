//
//  JadeMacRenderer.h
//  jade_mac
//
//  Created by Yushuo Liou on 1/27/14.
//  Copyright (c) 2014 ianslayer. All rights reserved.
//

#ifndef __jade_mac__JadeMacRenderer__
#define __jade_mac__JadeMacRenderer__

#ifdef __cplusplus
#include "../../../window.h"
#include <algorithm>

class MyInputListener : public InputListener
{
public:
    MyInputListener()
    {
        forward = false;
        backward = false;
        right = false;
        left = false;
        
        ClearState();
        mouseControl =false;
        
        cameraMoveSpeed = 50.f;
        cameraTurnSpeed = 2.f;
        
        frameCount = 0;
    }
    
    virtual void OnMouseMove(const int x, const int y, const int deltaX, const int deltaY)
    {
        rx += deltaX;
        ry += deltaY;
        
       // printf("mouse move, frame: %d\n", frameCount);
    }
    
    virtual void OnMouseButton(const int x, const int y, const MouseButton button, const bool pressed)
    {
        
        if(pressed && button == MOUSE_RIGHT)
        {
            mouseControl = true;
            
            //printf("mouse button down, frame: %d\n", frameCount);
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
        
        frameCount++;
    }
    
    bool forward;
    bool backward;
    bool right;
    bool left;
    
    bool mouseControl;
    float rx;
    float ry;
    
    float cameraMoveSpeed;
    float cameraTurnSpeed ;
    
    uint64_t frameCount;
    
};
#endif


#ifdef __cplusplus
extern "C" {
#endif
    void InputControl(float frameTime);
    void InitRenderer();
    void InitScene();
    void RenderFrame();
 
#ifdef __cplusplus
}
#endif

#endif /* defined(__jade_mac__JadeMacRenderer__) */
