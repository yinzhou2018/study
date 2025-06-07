#ifndef UTILS_H
#define UTILS_H

#include <CoreGraphics/CGWindow.h>
#include <CoreGraphics/CGImage.h>

namespace utils {

pid_t getActiveProcessID();
CGWindowID getActiveWindowID();
void saveImageToFile(CGImageRef imageRef, const char* filePath);

}  // namespace utils

#endif  // UTILS_H