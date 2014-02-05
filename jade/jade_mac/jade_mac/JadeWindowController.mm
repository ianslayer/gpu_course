//
//  JadeWindowController.m
//  jade_mac
//
//  Created by Yushuo Liou on 1/30/14.
//  Copyright (c) 2014 ianslayer. All rights reserved.
//

#import "JadeWindowController.h"
#include "JadeMacRenderer.h"

extern MyInputListener* inputListener;

@interface JadeWindowController ()
{
    MyInputListener* myInputListener;
    int lastX;
    int lastY;
}

@end

@implementation JadeWindowController

- (void) awakeFromNib
{
    [[self window] makeFirstResponder:self];
    [[self window] setAcceptsMouseMovedEvents:YES];
}

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {

        // Initialization code here.
        myInputListener = new MyInputListener();
        inputListener = myInputListener;
        lastX = 0;
        lastY = 0;
    }
    
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (void) keyDown:(NSEvent *)event
{
	unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];
    myInputListener->OnKey(c, TRUE);
    //printf("char: %c\n", c);
}

- (void) keyUp: (NSEvent*) event
{
   unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];
    myInputListener->OnKey(c, FALSE);
}

- (void) mouseMoved:(NSEvent *) event
{
    NSPoint curPoint = [event locationInWindow];
    //printf("current location: %f, %f\n", curPoint.x, curPoint.y);
    myInputListener->OnMouseMove(curPoint.x, curPoint.y, curPoint.x - lastX, -(curPoint.y - lastY));
    
    lastX = curPoint.x;
    lastY = curPoint.y;
}

- (void) mouseDragged: (NSEvent*) event
{
    NSPoint curPoint = [event locationInWindow];
    myInputListener->OnMouseMove(curPoint.x, curPoint.y, curPoint.x - lastX, -(curPoint.y - lastY));
    
    lastX = curPoint.x;
    lastY = curPoint.y;
}

- (void) rightMouseDragged: (NSEvent*) event
{
    NSPoint curPoint = [event locationInWindow];
    myInputListener->OnMouseMove(curPoint.x, curPoint.y, curPoint.x - lastX, -(curPoint.y - lastY));
    
    lastX = curPoint.x;
    lastY = curPoint.y;
}

- (void) mouseDown:(NSEvent *) event
{
    NSPoint curPoint = [event locationInWindow];
    //printf("pressed position: %f, %f\n", curPoint.x, curPoint.y);
    myInputListener->OnMouseButton(curPoint.x, curPoint.y, MOUSE_LEFT, TRUE);
}

- (void) mouseUp: (NSEvent *) event
{
    NSPoint curPoint = [event locationInWindow];
   // printf("released position: %f, %f\n", curPoint.x, curPoint.y);
    myInputListener->OnMouseButton(curPoint.x, curPoint.y, MOUSE_LEFT, FALSE);
}

- (void) otherMouseDown: (NSEvent*) event
{
    NSPoint curPoint = [event locationInWindow];
    //printf("pressed position: %f, %f\n", curPoint.x, curPoint.y);
    myInputListener->OnMouseButton(curPoint.x, curPoint.y, MOUSE_MIDDLE, TRUE);
}

- (void) otherMouseUp: (NSEvent*) event
{
    NSPoint curPoint = [event locationInWindow];
   // printf("released position: %f, %f\n", curPoint.x, curPoint.y);
    myInputListener->OnMouseButton(curPoint.x, curPoint.y, MOUSE_MIDDLE, FALSE);
}

- (void) rightMouseDown: (NSEvent*) event
{
    NSPoint curPoint = [event locationInWindow];
   // printf("pressed position: %f, %f\n", curPoint.x, curPoint.y);
    myInputListener->OnMouseButton(curPoint.x, curPoint.y, MOUSE_RIGHT, TRUE);
}

- (void) rightMouseUp: (NSEvent*) event
{
    NSPoint curPoint = [event locationInWindow];
   // printf("released position: %f, %f\n", curPoint.x, curPoint.y);
    myInputListener->OnMouseButton(curPoint.x, curPoint.y, MOUSE_RIGHT, FALSE);
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

@end
