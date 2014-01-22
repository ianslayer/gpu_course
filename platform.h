//
//  platform.h
//  jade_mac
//
//  Created by Yushuo Liou on 1/19/14.
//  Copyright (c) 2014 ianslayer. All rights reserved.
//

#ifndef JADE_PLATFORM_H
#define JADE_PLATFORM_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
    #include "Shlwapi.h"
    #pragma comment(lib, "Shlwapi.lib")
    #include <direct.h>
#elif defined(__APPLE__)
    #include <unistd.h>
    #define stricmp strcasecmp
#endif


#endif
