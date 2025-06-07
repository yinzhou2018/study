#ifndef UTILS_H
#define UTILS_H

#include <CoreGraphics/CGWindow.h>

namespace utils {

pid_t getActiveProcessID();
CGWindowID getActiveWindowID();

}  // namespace utils

#endif  // UTILS_H