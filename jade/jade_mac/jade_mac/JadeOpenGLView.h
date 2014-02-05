//
//  JadeOpenGLView.h
//  jade_mac
//
//  Created by Yushuo Liou on 1/24/14.
//  Copyright (c) 2014 ianslayer. All rights reserved.
//

#import <Cocoa/Cocoa.h>
//#import "JadeWindowController.h"

@class JadeWindowController;
@interface JadeOpenGLView : NSOpenGLView
{
    
    NSOpenGLContext* glContext;
    NSOpenGLPixelFormat* glPixelFormat;
    CVDisplayLinkRef displayLink;
    IBOutlet JadeWindowController* windowController;
}


-(void) drawRect:(NSRect)dirtyRect;

@end
