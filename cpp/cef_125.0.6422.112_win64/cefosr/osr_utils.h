
#ifndef CEF_CEFOSR_OSR_UTILS_H_
#define CEF_CEFOSR_OSR_UTILS_H_
#include <windows.h>

#include "include/internal/cef_types_wrappers.h"

namespace osr_utils {

int GetCefMouseModifiers(WPARAM wparam);
int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam);
bool IsKeyDown(WPARAM wparam);

constexpr int DPI_1X = 96;
// Returns the device scale factor for specified window. For example, 200% display scaling will
// return 2.0.
float GetDeviceScaleFactor(HWND hwnd);

int LogicalToDevice(int value, float device_scale_factor);
int DeviceToLogical(int value, float device_scale_factor);

bool IsMouseEventFromTouch(UINT message);

}  // namespace osr_utils

#endif  // CEF_CEFOSR_OSR_UTILS_H_