#include "screen_capture.h"
#include "sck_capture.h"

#include <ApplicationServices/ApplicationServices.h>

#include <chrono>
#include <iostream>

struct PerformanceCounter {
  PerformanceCounter(const char* name) : name_(name) { start_ = std::chrono::high_resolution_clock::now(); }
  ~PerformanceCounter() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
    std::cout << name_ << " spend: " << duration.count() << " ms" << std::endl;
  }

  std::chrono::high_resolution_clock::time_point start_;
  const char* name_;
};  // PerformanceCounter

void saveImageToFile(CGImageRef imageRef, const char* filePath) {
  CFURLRef url =
      CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8*)filePath, strlen(filePath), false);
  CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, nullptr);
  if (destination) {
    CGImageDestinationAddImage(destination, imageRef, nullptr);
    CGImageDestinationFinalize(destination);
    CFRelease(destination);
  }
  CFRelease(url);
}

void captureScreen(const char* filePath) {
  CGDirectDisplayID displayID = CGMainDisplayID();
  CGRect screenRect = CGDisplayBounds(displayID);
  CGImageRef imageRef = nullptr;
  {
    PerformanceCounter pc("captureScreen");
    imageRef = CGDisplayCreateImageForRect(displayID, screenRect);
  }
  if (imageRef) {
    saveImageToFile(imageRef, filePath);
    CGImageRelease(imageRef);
  }
}

void captureWindow(const char* filePath, CGWindowID windowID) {
  CGImageRef imageRef = nullptr;
  {
    PerformanceCounter pc("captureWindow");
    imageRef = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow, windowID, kCGWindowImageDefault);
  }
  if (imageRef) {
    saveImageToFile(imageRef, filePath);
    CGImageRelease(imageRef);
  }
}

CGWindowID getActiveWindowID() {
  pid_t activePID = -1;
  ProcessSerialNumber psn = {0, kCurrentProcess};
  if (GetFrontProcess(&psn) == noErr) {
    GetProcessPID(&psn, &activePID);
  }

  CGWindowID windowID = 0;

  CFArrayRef windowList = CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
  if (windowList) {
    CFIndex count = CFArrayGetCount(windowList);
    for (CFIndex i = 0; i < count; i++) {
      CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
      CFNumberRef windowNumberRef = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowNumber);
      CFNumberRef ownerPIDRef = (CFNumberRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID);

      pid_t windowPID = -1;
      CFNumberGetValue(ownerPIDRef, kCFNumberSInt32Type, &windowPID);

      if (windowPID == activePID && windowNumberRef) {
        CFNumberGetValue(windowNumberRef, kCFNumberSInt32Type, &windowID);
        break;
      }
    }
    CFRelease(windowList);
  }

  return windowID;
}

void screen_capture_test() {
  capture_with_sck();
  captureScreen("screen.png");
  captureWindow("window.png", getActiveWindowID());
}