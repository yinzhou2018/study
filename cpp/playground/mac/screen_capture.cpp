#include "screen_capture.h"
#include "sck_capture.h"
#include "utils.h"

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
  CFStringRef utType = CFSTR("public.png");
  CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, utType, 1, nullptr);
  if (destination) {
    CGImageDestinationAddImage(destination, imageRef, nullptr);
    CGImageDestinationFinalize(destination);
    CFRelease(destination);
  }

  CFRelease(url);
}

auto captureScreen() {
  CGDirectDisplayID displayID = CGMainDisplayID();
  CGRect screenRect = CGDisplayBounds(displayID);
  PerformanceCounter pc("captureScreen");
  return CGDisplayCreateImageForRect(displayID, screenRect);
}

auto captureWindow(CGWindowID windowID) {
  PerformanceCounter pc("captureWindow");
  return CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow, windowID, kCGWindowImageDefault);
}

auto captureWithSCK(CGWindowID windowID) {
  PerformanceCounter pc(windowID == 0 ? "captureScreenWithSCK" : "captureWindowWithSCK");
  return capture_with_sck(windowID);
}

void screen_capture_test() {
  auto activeWindowID = utils::getActiveWindowID();

  auto windowImageWithSck = captureWithSCK(activeWindowID);
  if (windowImageWithSck) {
    saveImageToFile(windowImageWithSck, "window_with_sck.png");
    CFRelease(windowImageWithSck);
  }

  auto windowImage = captureWindow(activeWindowID);
  if (windowImage) {
    saveImageToFile(windowImage, "window.png");
    CFRelease(windowImage);
  }

  auto screenImageWithSck = captureWithSCK(0);
  if (screenImageWithSck) {
    saveImageToFile(screenImageWithSck, "screen_with_sck.png");
    CFRelease(screenImageWithSck);
  }

  auto screenImage = captureScreen();
  if (screenImage) {
    saveImageToFile(screenImage, "screen.png");
    CFRelease(screenImage);
  }
}