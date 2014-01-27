//
//  JadeOpenGLView.h
//  jade_mac
//
//  Created by Yushuo Liou on 1/24/14.
//  Copyright (c) 2014 ianslayer. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface JadeOpenGLView : NSOpenGLView
{
    
    NSOpenGLContext* glContext;
    NSOpenGLPixelFormat* glPixelFormat;
    CVDisplayLinkRef displayLink;
}


-(void) drawRect:(NSRect)dirtyRect;

@end
