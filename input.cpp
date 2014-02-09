//
//  input.cpp
//  jade_mac
//
//  Created by Yushuo Liou on 1/29/14.
//  Copyright (c) 2014 ianslayer. All rights reserved.
//

#include "input.h"

struct KeyboardEvent
{
    int keyCode;
    int press;
    unsigned long long frame;
    unsigned long long timeStmp;
};

struct MouseEvent
{
    
};

#ifdef _WIN32_


#elif defined(__APPLE__)

#include <IOKit/hid/IOHIDLib.h>

IOHIDManagerRef gHidManager;

static CFMutableDictionaryRef hu_CreateDeviceMatchingDictionary(UInt32 inUsagePage, UInt32 inUsage)
{
    // create a dictionary to add usage page/usages to
    CFMutableDictionaryRef result = CFDictionaryCreateMutable(
                                                              kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (result) {
        if (inUsagePage) {
            // Add key for device type to refine the matching dictionary.
            CFNumberRef pageCFNumberRef = CFNumberCreate(
                                                         kCFAllocatorDefault, kCFNumberIntType, &inUsagePage);
            if (pageCFNumberRef) {
                CFDictionarySetValue(result,
                                     CFSTR(kIOHIDDeviceUsagePageKey), pageCFNumberRef);
                CFRelease(pageCFNumberRef);
                
                // note: the usage is only valid if the usage page is also defined
                if (inUsage) {
                    CFNumberRef usageCFNumberRef = CFNumberCreate(
                                                                  kCFAllocatorDefault, kCFNumberIntType, &inUsage);
                    if (usageCFNumberRef) {
                        CFDictionarySetValue(result,
                                             CFSTR(kIOHIDDeviceUsageKey), usageCFNumberRef);
                        CFRelease(usageCFNumberRef);
                    } else {
                        fprintf(stderr, "%s: CFNumberCreate(usage) failed.", __PRETTY_FUNCTION__);
                    }
                }
            } else {
                fprintf(stderr, "%s: CFNumberCreate(usage page) failed.", __PRETTY_FUNCTION__);
            }
        }
    } else {
        fprintf(stderr, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__);
    }
    return result;
}   // hu_CreateDeviceMatchingDictionary

// this will be called when the HID Manager matches a new (hot plugged) HID device
static void Handle_DeviceMatchingCallback(
                                          void *          inContext,       // context from IOHIDManagerRegisterDeviceMatchingCallback
                                          IOReturn        inResult,        // the result of the matching operation
                                          void *          inSender,        // the IOHIDManagerRef for the new device
                                          IOHIDDeviceRef  inIOHIDDeviceRef // the new HID device
) {
   // printf("%s(context: %p, result: %p, sender: %p, device: %p).\n",
    //       __PRETTY_FUNCTION__, inContext, (void *) inResult, inSender, (void*) inIOHIDDeviceRef);
}   // Handle_DeviceMatchingCallback

// this will be called when a HID device is removed (unplugged)
static void Handle_RemovalCallback(
                                   void *         inContext,       // context from IOHIDManagerRegisterDeviceMatchingCallback
                                   IOReturn       inResult,        // the result of the removing operation
                                   void *         inSender,        // the IOHIDManagerRef for the device being removed
                                   IOHIDDeviceRef inIOHIDDeviceRef // the removed HID device
) {
   // printf("%s(context: %p, result: %p, sender: %p, device: %p).\n",
    //       __PRETTY_FUNCTION__, inContext, (void *) inResult, inSender, (void*) inIOHIDDeviceRef);
}   // Handle_RemovalCallback

static void Handle_IOHIDInputValueCallback(
                                           void *          inContext,      // context from IOHIDManagerRegisterInputValueCallback
                                           IOReturn        inResult,       // completion result for the input value operation
                                           void *          inSender,       // the IOHIDManagerRef
                                           IOHIDValueRef   inIOHIDValueRef // the new element value
) {
  //  printf("%s(context: %p, result: %p, sender: %p, value: %p).\n",
     //      __PRETTY_FUNCTION__, inContext, (void *) inResult, inSender, (void*) inIOHIDValueRef);
    
    CFIndex valueLength = IOHIDValueGetLength(inIOHIDValueRef);
    const uint8_t* value = IOHIDValueGetBytePtr(inIOHIDValueRef);
    IOHIDElementRef element = IOHIDValueGetElement(inIOHIDValueRef);
    
    uint32_t elementUsage = IOHIDElementGetUsage(element);
    uint32_t elementUsagePage = IOHIDElementGetUsagePage(element);
    IOHIDElementType elementType = IOHIDElementGetType(element);
    CFStringRef elementName = IOHIDElementGetName(element);
    
    uint32_t reportCount = IOHIDElementGetReportCount(element);
   // printf("element report count: %d\n", reportCount);
    
    uint8_t buf[128];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, value, valueLength);
    
   // printf("element name: %s\n", CFStringGetCStringPtr(elementName, kCFStringEncodingMacRoman));
   // printf("element usage: %d\n", elementUsage);
   // printf("element usage page: %d\n", elementUsagePage);
   // printf("element type: %d\n", elementType);
   // printf("value: %d, length: %d\n", *value, valueLength);
}   // Handle_IOHIDInputValueCallback

void InitInput()
{
    gHidManager = IOHIDManagerCreate(CFAllocatorGetDefault(), kIOHIDOptionsTypeNone);
    
    // Create a matching dictionary
    CFDictionaryRef matchingCFDictRef =
    hu_CreateDeviceMatchingDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
    if (matchingCFDictRef) {
        // set the HID device matching dictionary
        IOHIDManagerSetDeviceMatching(gHidManager, matchingCFDictRef);
    } else {
        fprintf(stderr, "%s: hu_CreateDeviceMatchingDictionary failed.", __PRETTY_FUNCTION__);
        
        return;
    }
    
    IOHIDManagerRegisterDeviceMatchingCallback(gHidManager, Handle_DeviceMatchingCallback, nullptr);
    IOHIDManagerRegisterDeviceRemovalCallback(gHidManager, Handle_RemovalCallback, nullptr);
    IOHIDManagerScheduleWithRunLoop(gHidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    
    IOReturn tIOReturn = IOHIDManagerOpen(gHidManager, kIOHIDOptionsTypeNone);
    IOHIDManagerRegisterInputValueCallback(gHidManager, Handle_IOHIDInputValueCallback, nullptr);
}

#endif