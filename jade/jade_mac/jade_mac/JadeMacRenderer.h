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
extern "C" {
#endif
    void InputControl(float frameTime);
    void InitRenderer(float width, float height, float scaleFactor);
    void InitScene();
    void RenderFrame();
    
#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
#include "../../../window.h"
#include "../../../jade/renderer_gl.h"
#include "../../../jade/renderer_hw1.h"
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
        
        cameraMoveSpeed = 200.f;
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
        
        if(key == KEY_R && pressed == false)
        {
            options.reloadShaders = true;
        }
        
        if(key == KEY_0 && pressed == false)
        {
            options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_NONE;
        }
        
        if(key == KEY_1 && pressed == false )
        {
            options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_UV_TILING;
        }
        
        if(key == KEY_2 && pressed == false)
        {
            options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_TANGENT_SPACE;
        }
        
        if(key == KEY_3 && pressed == false)
        {
            options.dbgDraw =jade::GLRendererOptions::DBG_DRAW_DIFFUSE;
        }
        
        if(key == KEY_4 && pressed == false)
        {
            options.dbgDraw =jade::GLRendererOptions::DBG_DRAW_NORMAL;
        }
        
        if(key == KEY_5 && pressed == false)
        {
            options.dbgDraw =jade::GLRendererOptions::DBG_DRAW_SPECULAR;
        }
        
        if(key == KEY_6 && pressed == false)
        {
            options.dbgDraw =jade::GLRendererOptions::DBG_DRAW_DIFFUSE_LIGHTING;
        }
        
        if(key == KEY_7 && pressed == false)
        {
            options.dbgDraw =jade::GLRendererOptions::DBG_DRAW_SPECULAR_LIGHTING;
        }
        
        if(key == KEY_8 && pressed == false)
        {
            options.dbgDraw =jade::GLRendererOptions::DBG_DRAW_FRESNEL_SPECULAR_LIGHTING;
        }
		
		if(key == KEY_9 && pressed == false)
		{
			options.dbgDraw = jade::GLRendererOptions::DBG_DRAW_SHADOW_MAP;
		}
		if(key == KEY_V && pressed == false)
		{
			options.shadowTech = jade::GLRendererOptions::SHADOW_VARIANCE_SHADOW_MAP;
		}
		
		if(key == KEY_P && pressed == false)
		{
			options.shadowTech = jade::GLRendererOptions::SHADOW_MAP_PCF;
		}
		
		if(key == KEY_M && pressed == false)
		{
			options.screenShot = true;
		}
			
        
    }
    
    void ClearState()
    {
        rx = 0;
        ry = 0;
        
        options.reloadShaders = false;
        options.screenShot = false;
		
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
    
    jade::GLRendererOptions options;
    
};
#endif



#endif /* defined(__jade_mac__JadeMacRenderer__) */
