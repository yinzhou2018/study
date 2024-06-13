#include "osr_input_handler.h"
#include "osr_utils.h"
#include "osr_window.h"

#include <windowsx.h>
#include <iostream>

LRESULT OsrInputHandler::OnKeyMessage(UINT message, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (!owner_->browser_) {
    return 0;
  }

  CefKeyEvent event;
  event.windows_key_code = wParam;
  event.native_key_code = lParam;
  event.is_system_key = message == WM_SYSCHAR || message == WM_SYSKEYDOWN || message == WM_SYSKEYUP;

  if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
    event.type = KEYEVENT_RAWKEYDOWN;
  } else if (message == WM_KEYUP || message == WM_SYSKEYUP) {
    event.type = KEYEVENT_KEYUP;
  } else {
    event.type = KEYEVENT_CHAR;
  }
  event.modifiers = osr_utils::GetCefKeyboardModifiers(wParam, lParam);

  // mimic alt-gr check behaviour from
  // src/ui/events/win/events_win_utils.cc: GetModifiersFromKeyState
  if ((event.type == KEYEVENT_CHAR) && osr_utils::IsKeyDown(VK_RMENU)) {
    // reverse AltGr detection taken from PlatformKeyMap::UsesAltGraph
    // instead of checking all combination for ctrl-alt, just check current char
    HKL current_layout = ::GetKeyboardLayout(0);

    // https://docs.microsoft.com/en-gb/windows/win32/api/winuser/nf-winuser-vkkeyscanexw
    // ... high-order byte contains the shift state,
    // which can be a combination of the following flag bits.
    // 1 Either SHIFT key is pressed.
    // 2 Either CTRL key is pressed.
    // 4 Either ALT key is pressed.
    SHORT scan_res = ::VkKeyScanExW(wParam, current_layout);
    constexpr auto ctrlAlt = (2 | 4);
    if (((scan_res >> 8) & ctrlAlt) == ctrlAlt) {  // ctrl-alt pressed
      event.modifiers &= ~(EVENTFLAG_CONTROL_DOWN | EVENTFLAG_ALT_DOWN);
      event.modifiers |= EVENTFLAG_ALTGR_DOWN;
    }
  }

  owner_->browser_->GetHost()->SendKeyEvent(event);
  return 0;
}

LRESULT OsrInputHandler::OnMouseMessage(UINT message, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (!owner_->browser_ || osr_utils::IsMouseEventFromTouch(message)) {
    return 0;
  }

  DCHECK_GT(owner_->scale_factor_, 0);
  CefRefPtr<CefBrowserHost> browser_host = owner_->browser_->GetHost();

  int x = GET_X_LPARAM(lParam);
  int y = GET_Y_LPARAM(lParam);

  switch (message) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN: {
      ::SetCapture(owner_->hwnd_);

      auto btnType = (message == WM_LBUTTONDOWN ? MBT_LEFT : (message == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
      auto currentTime = GetMessageTime();
      ++last_click_count_;
      if (last_click_count_ > 1) {
        constexpr int kMaxClickCount = 3;
        auto cancelPreviousClick = (abs(last_click_x_ - x) > (GetSystemMetrics(SM_CXDOUBLECLK) / 2)) ||
                                   (abs(last_click_y_ - y) > (GetSystemMetrics(SM_CYDOUBLECLK) / 2)) ||
                                   ((currentTime - last_click_time_) > GetDoubleClickTime()) ||
                                   (last_click_count_ > kMaxClickCount) || (btnType != last_click_button_);
        if (cancelPreviousClick) {
          last_click_count_ = 1;
          last_click_x_ = x;
          last_click_y_ = y;
        }
      }
      last_click_time_ = currentTime;
      last_click_button_ = btnType;

      CefMouseEvent mouse_event;
      mouse_event.x = x;
      mouse_event.y = y;

      // TODO(Popup): Fix this.
      // last_mouse_down_on_view_ = !IsOverPopupWidget(x, y);
      // ApplyPopupOffset(mouse_event.x, mouse_event.y);

      mouse_event.x = osr_utils::DeviceToLogical(mouse_event.x, owner_->scale_factor_);
      mouse_event.y = osr_utils::DeviceToLogical(mouse_event.y, owner_->scale_factor_);
      mouse_event.modifiers = osr_utils::GetCefMouseModifiers(wParam);
      browser_host->SendMouseClickEvent(mouse_event, btnType, false, last_click_count_);
    } break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP: {
      if (::GetCapture() == owner_->hwnd_) {
        ::ReleaseCapture();
      }

      CefBrowserHost::MouseButtonType btnType =
          (message == WM_LBUTTONUP ? MBT_LEFT : (message == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));
      CefMouseEvent mouse_event;
      mouse_event.x = x;
      mouse_event.y = y;

      // TODO(Popup): Fix this.
      // if (last_mouse_down_on_view_ && IsOverPopupWidget(x, y) && (GetPopupXOffset() || GetPopupYOffset())) {
      //   break;
      // }
      // ApplyPopupOffset(mouse_event.x, mouse_event.y);

      mouse_event.x = osr_utils::DeviceToLogical(mouse_event.x, owner_->scale_factor_);
      mouse_event.y = osr_utils::DeviceToLogical(mouse_event.y, owner_->scale_factor_);
      mouse_event.modifiers = osr_utils::GetCefMouseModifiers(wParam);
      browser_host->SendMouseClickEvent(mouse_event, btnType, true, last_click_count_);
    } break;

    case WM_MOUSEMOVE: {
      if (!mouse_tracking_) {
        // Start tracking mouse leave. Required for the WM_MOUSELEAVE event to
        // be generated.
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = owner_->hwnd_;
        ::TrackMouseEvent(&tme);
        mouse_tracking_ = true;
      }

      CefMouseEvent mouse_event;
      mouse_event.x = x;
      mouse_event.y = y;

      // TODO(Popup): Fix this.
      // ApplyPopupOffset(mouse_event.x, mouse_event.y);

      mouse_event.x = osr_utils::DeviceToLogical(mouse_event.x, owner_->scale_factor_);
      mouse_event.y = osr_utils::DeviceToLogical(mouse_event.y, owner_->scale_factor_);
      mouse_event.modifiers = osr_utils::GetCefMouseModifiers(wParam);
      browser_host->SendMouseMoveEvent(mouse_event, false);
    } break;

    case WM_MOUSELEAVE: {
      if (mouse_tracking_) {
        // Stop tracking mouse leave.
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE & TME_CANCEL;
        tme.hwndTrack = owner_->hwnd_;
        ::TrackMouseEvent(&tme);
        mouse_tracking_ = false;
      }

      // Determine the cursor position in screen coordinates.
      POINT p;
      ::GetCursorPos(&p);
      ::ScreenToClient(owner_->hwnd_, &p);

      CefMouseEvent mouse_event;
      mouse_event.x = p.x;
      mouse_event.y = p.y;
      mouse_event.x = osr_utils::DeviceToLogical(mouse_event.x, owner_->scale_factor_);
      mouse_event.y = osr_utils::DeviceToLogical(mouse_event.y, owner_->scale_factor_);
      mouse_event.modifiers = osr_utils::GetCefMouseModifiers(wParam);
      browser_host->SendMouseMoveEvent(mouse_event, true);
    } break;

    case WM_MOUSEWHEEL: {
      POINT screen_point = {x, y};
      HWND scrolled_wnd = ::WindowFromPoint(screen_point);
      if (scrolled_wnd != owner_->hwnd_) {
        break;
      }

      ::ScreenToClient(owner_->hwnd_, &screen_point);
      int delta = GET_WHEEL_DELTA_WPARAM(wParam);

      CefMouseEvent mouse_event;
      mouse_event.x = screen_point.x;
      mouse_event.y = screen_point.y;

      // TODO(Popup): Fix this.
      // ApplyPopupOffset(mouse_event.x, mouse_event.y);

      mouse_event.x = osr_utils::DeviceToLogical(mouse_event.x, owner_->scale_factor_);
      mouse_event.y = osr_utils::DeviceToLogical(mouse_event.y, owner_->scale_factor_);
      mouse_event.modifiers = osr_utils::GetCefMouseModifiers(wParam);
      browser_host->SendMouseWheelEvent(mouse_event, osr_utils::IsKeyDown(VK_SHIFT) ? delta : 0,
                                        !osr_utils::IsKeyDown(VK_SHIFT) ? delta : 0);
    } break;
  }

  return 0;
}

LRESULT OsrInputHandler::OnTouchMessage(UINT message, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (!owner_->browser_) {
    return 0;
  }

  int num_points = LOWORD(wParam);
  // Chromium only supports upto 16 touch points.
  if (num_points < 0 || num_points > 16) {
    return 0;
  }
  std::unique_ptr<TOUCHINPUT[]> input(new TOUCHINPUT[num_points]);
  if (GetTouchInputInfo(reinterpret_cast<HTOUCHINPUT>(lParam), num_points, input.get(), sizeof(TOUCHINPUT))) {
    CefTouchEvent touch_event;
    for (int i = 0; i < num_points; ++i) {
      POINT point;
      point.x = TOUCH_COORD_TO_PIXEL(input[i].x);
      point.y = TOUCH_COORD_TO_PIXEL(input[i].y);

      // Platforms below Windows 8 not supported.
      // if (!IsWindows_8_Or_Newer()) {
      //   // Windows 7 sends touch events for touches in the non-client area,
      //   // whereas Windows 8 does not. In order to unify the behaviour, always
      //   // ignore touch events in the non-client area.
      //   LPARAM l_param_ht = MAKELPARAM(point.x, point.y);
      //   LRESULT hittest = SendMessage(hwnd_, WM_NCHITTEST, 0, l_param_ht);
      //   if (hittest != HTCLIENT) {
      //     return false;
      //   }
      // }

      ScreenToClient(owner_->hwnd_, &point);
      touch_event.x = osr_utils::DeviceToLogical(point.x, owner_->scale_factor_);
      touch_event.y = osr_utils::DeviceToLogical(point.y, owner_->scale_factor_);

      // Touch point identifier stays consistent in a touch contact sequence
      touch_event.id = input[i].dwID;

      if (input[i].dwFlags & TOUCHEVENTF_DOWN) {
        touch_event.type = CEF_TET_PRESSED;
      } else if (input[i].dwFlags & TOUCHEVENTF_MOVE) {
        touch_event.type = CEF_TET_MOVED;
      } else if (input[i].dwFlags & TOUCHEVENTF_UP) {
        touch_event.type = CEF_TET_RELEASED;
      }

      touch_event.radius_x = 0;
      touch_event.radius_y = 0;
      touch_event.rotation_angle = 0;
      touch_event.pressure = 0;
      touch_event.modifiers = 0;

      owner_->browser_->GetHost()->SendTouchEvent(touch_event);
    }
    CloseTouchInputHandle(reinterpret_cast<HTOUCHINPUT>(lParam));
    handled = true;
    return 0;
  }

  return 0;
}

LRESULT OsrInputHandler::OnCaptureChanged(UINT message, WPARAM wParam, LPARAM lParam, bool& handled) {
  OnCaptureLost();
  return 0;
}

LRESULT OsrInputHandler::OnCancelMode(UINT message, WPARAM wParam, LPARAM lParam, bool& handled) {
  OnCaptureLost();
  return 0;
}

void OsrInputHandler::OnCaptureLost() {
  if (owner_->browser_) {
    owner_->browser_->GetHost()->SendCaptureLostEvent();
  }
}