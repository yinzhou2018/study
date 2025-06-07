#import "utils.h"
#import <Cocoa/Cocoa.h>

namespace utils {

pid_t getActiveProcessID() {
  pid_t activePID = -1;
  NSRunningApplication* activeApp = [[NSWorkspace sharedWorkspace] frontmostApplication];
  if (activeApp) {
    activePID = [activeApp processIdentifier];
  }
  return activePID;
}

CGWindowID getActiveWindowID() {
  pid_t activePID = getActiveProcessID();
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

}  // namespace utils
