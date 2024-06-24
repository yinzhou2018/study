#include "osr_window.h"
#include "osr_ime_handler.h"
#include "osr_input_handler.h"
#include "osr_render_handler_d3d11.h"
#include "osr_render_handler_gl.h"
#include "osr_utils.h"

#include "include/base/cef_bind.h"
#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"

#include <windowsx.h>
#include <functional>
#include <iostream>

#pragma comment(lib, "Msimg32.lib")

#define WM_ASYNC_CALL_MESSAGE (WM_USER + 1)
using AsyncCallFunction = std::function<void()>;

using namespace std::placeholders;
OsrWindow* g_win_ = nullptr;

LRESULT CALLBACK OsrWindow::s_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  static OsrWindow* window = nullptr;
  if (WM_NCCREATE == msg) {
    CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
    window = reinterpret_cast<OsrWindow*>(cs->lpCreateParams);
    window->hwnd_ = hwnd;
    window->scale_factor_ = osr_utils::GetDeviceScaleFactor(window->hwnd_);
    window->ime_handler_ = std::make_unique<OsrImeHandler>(window);
    window->input_handler_ = std::make_unique<OsrInputHandler>(window);
  }
  return window ? window->WindowProc(hwnd, msg, wParam, lParam) : ::DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK OsrWindow::s_getmessage_hook_proc(int code, WPARAM wParam, LPARAM lParam) {
  if (code == HC_ACTION && g_win_ && g_win_->ime_handler_) {
    MSG* msg = reinterpret_cast<MSG*>(lParam);
    if ((VK_PROCESSKEY == msg->wParam) && (WM_KEYDOWN == msg->message)) {
      // 更早hook到WM_KEYDOWN消息，才能通过`ImmGetVirtualKey`获取到真实的虚拟键码，
      // 详见MSDN: https://learn.microsoft.com/en-us/windows/win32/api/imm/nf-imm-immgetvirtualkey
      // 解决搜狗输入法没按标准规范操作带来的问题
      g_win_->ime_handler_->OnImeKeydown(::ImmGetVirtualKey(g_win_->hwnd_));
    }
  }
  return ::CallNextHookEx(nullptr, code, wParam, lParam);
}

CefRefPtr<OsrWindow> OsrWindow::Create(const OsrRendererSettings& settings) {
  WNDCLASSEX wcex = {0};
  LPCWSTR window_class = L"OSRWindowClass";

  wcex.cbSize = sizeof(wcex);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = s_window_proc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = GetModuleHandle(nullptr);
  wcex.hIcon = nullptr;
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH));
  wcex.lpszMenuName = nullptr;
  wcex.lpszClassName = window_class;
  wcex.hIconSm = nullptr;
  ::RegisterClassEx(&wcex);

  OsrWindow* window = new OsrWindow();
  window->settings_ = settings;
  auto hwnd = ::CreateWindowExW(0, window_class, L"OSRWindow", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, window);

  if (RHIType::RT_D3D11 == settings.rhi_type) {
    window->render_handler_ = std::make_unique<OsrRenderHandlerD3D11>(settings, hwnd);
  } else {
    window->render_handler_ = std::make_unique<OsrRenderHandlerGL>(settings, hwnd);
  }

  g_win_ = window;
  window->hook_ = ::SetWindowsHookEx(WH_GETMESSAGE, OsrWindow::s_getmessage_hook_proc, nullptr, ::GetCurrentThreadId());

  return window;
}

void OsrWindow::Show() {
  ShowWindow(hwnd_, SW_SHOW);
}

void OsrWindow::CreateBrowser(const std::string& url) {
  CefBrowserSettings browser_settings;
  browser_settings.image_loading = STATE_ENABLED;
  browser_settings.javascript = STATE_ENABLED;
  browser_settings.javascript_access_clipboard = STATE_ENABLED;
  browser_settings.local_storage = STATE_ENABLED;
  browser_settings.remote_fonts = STATE_ENABLED;
  browser_settings.webgl = STATE_ENABLED;
  browser_settings.windowless_frame_rate = settings_.frame_rate;
  browser_settings.background_color = settings_.background_color;

  CefWindowInfo window_info;
  window_info.shared_texture_enabled = settings_.shared_texture_enabled;
  window_info.SetAsWindowless(hwnd_);

  CefBrowserHost::CreateBrowser(window_info, this, url, browser_settings, nullptr, nullptr);
}

LRESULT OsrWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  using MSGHandler = std::function<LRESULT(UINT, WPARAM, LPARAM, bool&)>;
  using MSGHandlerEntry = std::pair<UINT, MSGHandler>;
  static const MSGHandlerEntry s_msg_handlers[] = {
      {WM_SIZE, std::bind(&OsrWindow::OnSized, this, _1, _2, _3, _4)},
      {WM_PAINT, std::bind(&OsrWindow::OnNativePaint, this, _1, _2, _3, _4)},
      // {WM_ERASEBKGND, std::bind(&OsrWindow::OnEraseBackground, this, _1, _2, _3, _4)},
      {WM_DPICHANGED, std::bind(&OsrWindow::OnDPIChanged, this, _1, _2, _3, _4)},
      {WM_CLOSE, std::bind(&OsrWindow::OnClose, this, _1, _2, _3, _4)},
      {WM_SETFOCUS, std::bind(&OsrWindow::OnFocusMessage, this, _1, _2, _3, _4)},
      {WM_ASYNC_CALL_MESSAGE, std::bind(&OsrWindow::OnAsyncCallMessage, this, _1, _2, _3, _4)},
      {WM_MOVE, std::bind(&OsrWindow::OnMove, this, _1, _2, _3, _4)},
      {WM_MOVING, std::bind(&OsrWindow::OnMove, this, _1, _2, _3, _4)},
      {WM_KEYDOWN, std::bind(&OsrInputHandler::OnKeyMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_KEYUP, std::bind(&OsrInputHandler::OnKeyMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_CHAR, std::bind(&OsrInputHandler::OnKeyMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_SYSKEYDOWN, std::bind(&OsrInputHandler::OnKeyMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_SYSKEYUP, std::bind(&OsrInputHandler::OnKeyMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_SYSCHAR, std::bind(&OsrInputHandler::OnKeyMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_LBUTTONDOWN, std::bind(&OsrInputHandler::OnMouseMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_RBUTTONDOWN, std::bind(&OsrInputHandler::OnMouseMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_MBUTTONDOWN, std::bind(&OsrInputHandler::OnMouseMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_LBUTTONUP, std::bind(&OsrInputHandler::OnMouseMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_RBUTTONUP, std::bind(&OsrInputHandler::OnMouseMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_MBUTTONUP, std::bind(&OsrInputHandler::OnMouseMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_MOUSEMOVE, std::bind(&OsrInputHandler::OnMouseMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_MOUSELEAVE, std::bind(&OsrInputHandler::OnMouseMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_MOUSEWHEEL, std::bind(&OsrInputHandler::OnMouseMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_TOUCH, std::bind(&OsrInputHandler::OnTouchMessage, input_handler_.get(), _1, _2, _3, _4)},
      {WM_CAPTURECHANGED, std::bind(&OsrInputHandler::OnCaptureChanged, input_handler_.get(), _1, _2, _3, _4)},
      {WM_CANCELMODE, std::bind(&OsrInputHandler::OnCancelMode, input_handler_.get(), _1, _2, _3, _4)},
      {WM_IME_SETCONTEXT, std::bind(&OsrImeHandler::OnIMESetContext, ime_handler_.get(), _1, _2, _3, _4)},
      {WM_IME_STARTCOMPOSITION, std::bind(&OsrImeHandler::OnIMEStartComposition, ime_handler_.get(), _1, _2, _3, _4)},
      {WM_IME_COMPOSITION, std::bind(&OsrImeHandler::OnIMEComposition, ime_handler_.get(), _1, _2, _3, _4)},
      {WM_IME_ENDCOMPOSITION, std::bind(&OsrImeHandler::OnIMEEndComposition, ime_handler_.get(), _1, _2, _3, _4)},
  };  // s_msg_handlers

  auto iter = std::find_if(std::begin(s_msg_handlers), std::end(s_msg_handlers),
                           [msg](const MSGHandlerEntry& e) { return e.first == msg; });
  if (iter != std::end(s_msg_handlers)) {
    bool handled = false;
    LRESULT result = (iter->second)(msg, wParam, lParam, handled);
    if (handled) {
      return result;
    }
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT OsrWindow::OnSized(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (browser_) {
    if (wParam != SIZE_MINIMIZED) {
      browser_->GetHost()->WasResized();
      if (browser_hidden_) {
        browser_hidden_ = false;
        browser_->GetHost()->WasHidden(false);
      }
    } else {
      browser_->GetHost()->WasHidden(true);
      browser_hidden_ = true;
    }
  }
  return 0;
}

LRESULT OsrWindow::OnNativePaint(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  // 随便绘制一段文本，测试基于DirectComposition的alpha合成效果
  PAINTSTRUCT ps;
  auto dc = BeginPaint(hwnd_, &ps);
  RECT client_rect = {0};
  ::GetClientRect(hwnd_, &client_rect);
  std::wstring text = L"注意：我是基于GDI绘制的原生内容！";
  ::SetBkMode(dc, TRANSPARENT);
  ::DrawTextExW(dc, (LPWSTR)text.c_str(), (int)text.size(), &client_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE,
                nullptr);
  EndPaint(hwnd_, &ps);

  if (browser_) {
    browser_->GetHost()->Invalidate(PET_VIEW);
  }

  handled = true;
  return 0;
}

LRESULT OsrWindow::OnEraseBackground(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  handled = true;
  return browser_ != nullptr;
}

LRESULT OsrWindow::OnDPIChanged(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (LOWORD(wParam) != HIWORD(wParam)) {
    NOTIMPLEMENTED() << "Received non-square scaling factors";
    return 0;
  }

  auto factor = static_cast<float>(LOWORD(wParam)) / osr_utils::DPI_1X;
  if (factor == scale_factor_) {
    return 0;
  }

  scale_factor_ = factor;
  if (browser_) {
    browser_->GetHost()->NotifyScreenInfoChanged();
  }

  // Suggested size and position of the current window scaled for the new DPI.
  const RECT* rect = reinterpret_cast<RECT*>(lParam);
  ::SetWindowPos(hwnd_, nullptr, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
                 SWP_NOZORDER);

  return 0;
}

LRESULT OsrWindow::OnClose(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (hook_) {
    ::UnhookWindowsHookEx(hook_);
    hook_ = nullptr;
  }

  if (browser_) {
    browser_->GetHost()->CloseBrowser(true);
  } else {
    CefQuitMessageLoop();
  }

  return 0;
}

LRESULT OsrWindow::OnMove(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  // Notify the browser of move events so that popup windows are displayed
  // in the correct location and dismissed when the window moves.
  if (browser_) {
    browser_->GetHost()->NotifyMoveOrResizeStarted();
  }
  return 0;
}

LRESULT OsrWindow::OnFocusMessage(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (browser_) {
    browser_->GetHost()->SetFocus(msg == WM_SETFOCUS);
  }

  return 0;
}

LRESULT OsrWindow::OnAsyncCallMessage(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  handled = true;
  auto fn = reinterpret_cast<std::function<void()>*>(wParam);
  (*fn)();
  delete fn;
  return 0;
}

void OsrWindow::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefContextMenuParams> params,
                                    CefRefPtr<CefMenuModel> model) {
  // 仅保留编辑区域的菜单
  if (!params->IsEditable()) {
    model->Clear();
  }
}

bool OsrWindow::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefProcessId source_process,
                                         CefRefPtr<CefProcessMessage> message) {
  if (message->GetName() == kLastFocusedIsEditableMessage && ime_handler_) {
    if (IsOnMainThread()) {
      ime_handler_->EnableIME();
    } else {
      AsyncCallFunction* fn = new AsyncCallFunction(std::bind(&OsrImeHandler::EnableIME, ime_handler_.get()));
      ::PostMessage(hwnd_, WM_ASYNC_CALL_MESSAGE, reinterpret_cast<WPARAM>(fn), 0);
    }
  } else if (message->GetName() == kLastFocusedIsNotEditableMessage && ime_handler_) {
    if (IsOnMainThread()) {
      ime_handler_->DisableIME();
    } else {
      AsyncCallFunction* fn = new AsyncCallFunction(std::bind(&OsrImeHandler::DisableIME, ime_handler_.get()));
      ::PostMessage(hwnd_, WM_ASYNC_CALL_MESSAGE, reinterpret_cast<WPARAM>(fn), 0);
    }
  }

  return true;
}

void OsrWindow::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  browser_ = browser;
  render_handler_->SetBrowser(browser);
}

void OsrWindow::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  browser_ = nullptr;
  render_handler_.reset();
  CefQuitMessageLoop();
  if (!IsOnMainThread()) {
    ::PostThreadMessage(created_thread_id_, WM_QUIT, 0, 0);
  }
}

void OsrWindow::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) {
  if (frame->IsMain()) {
    CefWindowInfo win_info;
    CefBrowserSettings settings;
    win_info.SetAsPopup(nullptr, "DevTools");
    browser->GetHost()->ShowDevTools(win_info, nullptr, settings, CefPoint());
  }
}

bool OsrWindow::OnCursorChange(CefRefPtr<CefBrowser> browser,
                               CefCursorHandle cursor,
                               cef_cursor_type_t type,
                               const CefCursorInfo& custom_cursor_info) {
  // Change the window's cursor.
  SetClassLongPtr(hwnd_, GCLP_HCURSOR, static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
  ::SetCursor(cursor);
  return true;
}

bool OsrWindow::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) {
  // TODO(Tooptip): Not implemented.
  return true;
}

bool OsrWindow::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) {
  DCHECK_GT(scale_factor_, 0);
  screen_info.device_scale_factor = scale_factor_;

  CefRect view_rect;
  GetViewRect(browser, view_rect);

  // The screen info rectangles are used by the renderer to create and position
  // popups. Keep popups inside the view rectangle.
  screen_info.rect = view_rect;
  screen_info.available_rect = view_rect;

  return true;
}

void OsrWindow::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
  DCHECK_GT(scale_factor_, 0);

  RECT client_rect = {0};
  ::GetClientRect(hwnd_, &client_rect);

  rect.x = rect.y = 0;
  rect.width = osr_utils::DeviceToLogical(client_rect.right - client_rect.left, scale_factor_);
  if (rect.width == 0) {
    rect.width = 1;
  }
  rect.height = osr_utils::DeviceToLogical(client_rect.bottom - client_rect.top, scale_factor_);
  if (rect.height == 0) {
    rect.height = 1;
  }
}

bool OsrWindow::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY) {
  DCHECK_GT(scale_factor_, 0);

  POINT pt{osr_utils::LogicalToDevice(viewX, scale_factor_), osr_utils::LogicalToDevice(viewY, scale_factor_)};
  ClientToScreen(hwnd_, &pt);
  screenX = pt.x;
  screenY = pt.y;
  return true;
}

struct PerformanceTracker {
  static int last_render_time_;
  static int last_call_time_;
  int now_time_;
  PerformanceTracker(const char* tag) {
    now_time_ = GetTickCount64();
    if (last_call_time_ == 0) {
      last_call_time_ = now_time_;
    } else {
      auto interval = now_time_ - last_call_time_;
      // if (interval > 30) {
        std::cout << "[" << tag << "]Interval between paints: " << interval
                  << "ms, last render cost time: " << last_render_time_ << "\n";
      // }
      last_call_time_ = now_time_;
    }
  }

  ~PerformanceTracker() { last_render_time_ = ::GetTickCount64() - now_time_; }
};  // PerformanceTracker

int PerformanceTracker::last_render_time_ = 0;
int PerformanceTracker::last_call_time_ = 0;

void OsrWindow::OnPaint(CefRefPtr<CefBrowser> browser,
                        PaintElementType type,
                        const RectList& dirtyRects,
                        const void* buffer,
                        int width,
                        int height) {
  PerformanceTracker tracker("OnPaint");
  DCHECK(render_handler_);
  render_handler_->OnPaint(browser, type, dirtyRects, buffer, width, height);
  // HDC hdc = ::GetDC(hwnd_);

  // BITMAPINFO bmi = {};
  // bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  // bmi.bmiHeader.biWidth = width;
  // bmi.bmiHeader.biHeight = -height;  // top-down
  // bmi.bmiHeader.biPlanes = 1;
  // bmi.bmiHeader.biBitCount = 32;
  // bmi.bmiHeader.biCompression = BI_RGB;
  // auto bmp =
  //     ::CreateDIBitmap(hdc, reinterpret_cast<const BITMAPINFOHEADER*>(&bmi), CBM_INIT, buffer, &bmi, DIB_RGB_COLORS);
  // auto mem_dc = ::CreateCompatibleDC(hdc);
  // ::SelectObject(mem_dc, bmp);

  // BLENDFUNCTION blendFunc;
  // blendFunc.BlendOp = AC_SRC_OVER;
  // blendFunc.BlendFlags = 0;
  // blendFunc.SourceConstantAlpha = 255;
  // blendFunc.AlphaFormat = AC_SRC_ALPHA;

  // ::AlphaBlend(hdc, 0, 0, width, height, mem_dc, 0, 0, width, height, blendFunc);

  // ::SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, buffer, &bmi, DIB_RGB_COLORS);
  // ::SelectObject(mem_dc, nullptr);
  // ::DeleteObject(bmp);
  // ::DeleteDC(mem_dc);
  // ::ReleaseDC(hwnd_, hdc);
}

void OsrWindow::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                   PaintElementType type,
                                   const RectList& dirtyRects,
                                   const CefAcceleratedPaintInfo& info) {
  PerformanceTracker tracker("OnAcceleratedPaint");
  DCHECK(render_handler_);
  render_handler_->OnAcceleratedPaint(browser, type, dirtyRects, info);
}

void OsrWindow::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) {
  render_handler_->OnPopupShow(browser, show);
}

void OsrWindow::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) {
  CefRect device_rect = {
      osr_utils::LogicalToDevice(rect.x, scale_factor_), osr_utils::LogicalToDevice(rect.y, scale_factor_),
      osr_utils::LogicalToDevice(rect.width, scale_factor_), osr_utils::LogicalToDevice(rect.height, scale_factor_)};
  render_handler_->OnPopupSize(browser, device_rect);
}

void OsrWindow::OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,
                                             const CefRange& selected_range,
                                             const RectList& character_bounds) {
  if (ime_handler_) {
    if (IsOnMainThread()) {
      ime_handler_.get()->OnImeCompositionRangeChanged(selected_range, character_bounds);
    } else {
      AsyncCallFunction* fn = new AsyncCallFunction(std::bind(&OsrImeHandler::OnImeCompositionRangeChanged,
                                                              ime_handler_.get(), selected_range, character_bounds));
      ::PostMessage(hwnd_, WM_ASYNC_CALL_MESSAGE, reinterpret_cast<WPARAM>(fn), 0);
    }
  }
}