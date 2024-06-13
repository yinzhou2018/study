#include "osr_utils.h"

#include <ShellScalingApi.h>
#include <cmath>

#include <include/base/cef_logging.h>

#pragma comment(lib, "Shcore.lib")

namespace osr_utils {

int GetCefMouseModifiers(WPARAM wparam) {
  int modifiers = 0;
  if (wparam & MK_CONTROL) {
    modifiers |= EVENTFLAG_CONTROL_DOWN;
  }
  if (wparam & MK_SHIFT) {
    modifiers |= EVENTFLAG_SHIFT_DOWN;
  }
  if (IsKeyDown(VK_MENU)) {
    modifiers |= EVENTFLAG_ALT_DOWN;
  }
  if (wparam & MK_LBUTTON) {
    modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
  }
  if (wparam & MK_MBUTTON) {
    modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
  }
  if (wparam & MK_RBUTTON) {
    modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
  }

  // Low bit set from GetKeyState indicates "toggled".
  if (::GetKeyState(VK_NUMLOCK) & 1) {
    modifiers |= EVENTFLAG_NUM_LOCK_ON;
  }
  if (::GetKeyState(VK_CAPITAL) & 1) {
    modifiers |= EVENTFLAG_CAPS_LOCK_ON;
  }
  return modifiers;
}

int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) {
  int modifiers = 0;
  if (IsKeyDown(VK_SHIFT)) {
    modifiers |= EVENTFLAG_SHIFT_DOWN;
  }
  if (IsKeyDown(VK_CONTROL)) {
    modifiers |= EVENTFLAG_CONTROL_DOWN;
  }
  if (IsKeyDown(VK_MENU)) {
    modifiers |= EVENTFLAG_ALT_DOWN;
  }

  // Low bit set from GetKeyState indicates "toggled".
  if (::GetKeyState(VK_NUMLOCK) & 1) {
    modifiers |= EVENTFLAG_NUM_LOCK_ON;
  }
  if (::GetKeyState(VK_CAPITAL) & 1) {
    modifiers |= EVENTFLAG_CAPS_LOCK_ON;
  }

  switch (wparam) {
    case VK_RETURN:
      if ((lparam >> 16) & KF_EXTENDED) {
        modifiers |= EVENTFLAG_IS_KEY_PAD;
      }
      break;
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
      if (!((lparam >> 16) & KF_EXTENDED)) {
        modifiers |= EVENTFLAG_IS_KEY_PAD;
      }
      break;
    case VK_NUMLOCK:
    case VK_NUMPAD0:
    case VK_NUMPAD1:
    case VK_NUMPAD2:
    case VK_NUMPAD3:
    case VK_NUMPAD4:
    case VK_NUMPAD5:
    case VK_NUMPAD6:
    case VK_NUMPAD7:
    case VK_NUMPAD8:
    case VK_NUMPAD9:
    case VK_DIVIDE:
    case VK_MULTIPLY:
    case VK_SUBTRACT:
    case VK_ADD:
    case VK_DECIMAL:
    case VK_CLEAR:
      modifiers |= EVENTFLAG_IS_KEY_PAD;
      break;
    case VK_SHIFT:
      if (IsKeyDown(VK_LSHIFT)) {
        modifiers |= EVENTFLAG_IS_LEFT;
      } else if (IsKeyDown(VK_RSHIFT)) {
        modifiers |= EVENTFLAG_IS_RIGHT;
      }
      break;
    case VK_CONTROL:
      if (IsKeyDown(VK_LCONTROL)) {
        modifiers |= EVENTFLAG_IS_LEFT;
      } else if (IsKeyDown(VK_RCONTROL)) {
        modifiers |= EVENTFLAG_IS_RIGHT;
      }
      break;
    case VK_MENU:
      if (IsKeyDown(VK_LMENU)) {
        modifiers |= EVENTFLAG_IS_LEFT;
      } else if (IsKeyDown(VK_RMENU)) {
        modifiers |= EVENTFLAG_IS_RIGHT;
      }
      break;
    case VK_LWIN:
      modifiers |= EVENTFLAG_IS_LEFT;
      break;
    case VK_RWIN:
      modifiers |= EVENTFLAG_IS_RIGHT;
      break;
  }
  return modifiers;
}

bool IsKeyDown(WPARAM wparam) {
  return (GetKeyState(wparam) & 0x8000) != 0;
}

// Returns the device scale factor for specified window. For example, 200% display scaling will
// return 2.0.
// Note that this function is not supported on platforms below Windows 8.1
float GetDeviceScaleFactor(HWND hwnd) {
  auto monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
  UINT dpi_x = DPI_1X, dpi_y = DPI_1X;
  GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);

  if (dpi_x != dpi_y) {
    NOTIMPLEMENTED() << "Received non-square scaling factors";
  }

  return static_cast<float>(dpi_x) / DPI_1X;
}

int LogicalToDevice(int value, float device_scale_factor) {
  float scaled_val = static_cast<float>(value) * device_scale_factor;
  return static_cast<int>(std::floor(scaled_val));
}

int DeviceToLogical(int value, float device_scale_factor) {
  float scaled_val = static_cast<float>(value) / device_scale_factor;
  return static_cast<int>(std::floor(scaled_val));
}

// Helper function to detect mouse messages coming from emulation of touch
// events. These should be ignored.
bool IsMouseEventFromTouch(UINT message) {
#define MOUSEEVENTF_FROMTOUCH 0xFF515700
  return (message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST) &&
         (GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH;
}

}  // namespace osr_utils