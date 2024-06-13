
#ifndef CEF_OSR_INPUT_HANDLER_H_
#define CEF_OSR_INPUT_HANDLER_H_
#include <windows.h>
#include "include/cef_client.h"

class OsrWindow;

class OsrInputHandler {
 public:
  OsrInputHandler(OsrWindow* window) : owner_(window){};

  LRESULT OnKeyMessage(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnMouseMessage(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnTouchMessage(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnCaptureChanged(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnCancelMode(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

 private:
  void OnCaptureLost();

  OsrWindow* owner_;

  // Mouse state tracking.
  POINT last_mouse_pos_;
  POINT current_mouse_pos_;
  bool mouse_rotation_ = false;
  bool mouse_tracking_ = false;
  int last_click_x_ = 0;
  int last_click_y_ = 0;
  CefBrowserHost::MouseButtonType last_click_button_ = MBT_LEFT;
  int last_click_count_ = 1;
  double last_click_time_ = 0;
  bool last_mouse_down_on_view_ = false;
};  // OsrInputHandler

#endif  // CEF_OSR_INPUT_HANDLER_H_