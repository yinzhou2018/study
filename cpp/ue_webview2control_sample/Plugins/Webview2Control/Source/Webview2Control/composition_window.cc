#include "composition_window.h"
#include "js_callable_object.h"

#include <WebView2EnvironmentOptions.h>
#include <windowsx.h>
#include <wrl/event.h>
#include <cassert>
#include <sstream>

using namespace Microsoft::WRL::Details;

LRESULT CALLBACK CompositionWindow::s_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  auto window = reinterpret_cast<CompositionWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  return window ? window->WindowProc(hwnd, msg, wParam, lParam) : ::DefWindowProc(hwnd, msg, wParam, lParam);
}

void CompositionWindow::Initialize(HWND target_window, const RECT& composition_screen_rect, const std::wstring& url) {
  POINT pt = {composition_screen_rect.left, composition_screen_rect.top};
  ::ScreenToClient(target_window, &pt);
  webview_rect_ = {pt.x, pt.y, pt.x + composition_screen_rect.right - composition_screen_rect.left,
                   pt.y + composition_screen_rect.bottom - composition_screen_rect.top};
  InitializeImpl(target_window, url);
}

void CompositionWindow::Initialize(HWND target_window, const std::wstring& url) {
  use_client_area_rect_ = true;
  ::GetClientRect(target_window, &webview_rect_);
  InitializeImpl(target_window, url);
}

void CompositionWindow::Shutdown() {
  Cleanup();
}

void CompositionWindow::InitializeImpl(HWND target_window, const std::wstring& url) {
  url_ = url;
  hwnd_ = target_window;

  // 没有`CS_DBLCLKS`标志无法发出DBCLICK消息
  auto class_style = ::GetClassLongPtrW(hwnd_, GCL_STYLE);
  assert(class_style & CS_DBLCLKS != 0);

  ::SetWindowLongPtrW(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  original_window_proc_ = reinterpret_cast<WINDOW_PROC>(
      ::SetWindowLongPtrW(hwnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(s_window_proc)));

  CreateWebView();
}

void CompositionWindow::ReiszeCompositionRect(const RECT& composition_screen_rect) {
  POINT pt = {composition_screen_rect.left, composition_screen_rect.top};
  ::ScreenToClient(hwnd_, &pt);
  webview_rect_ = {pt.x, pt.y, pt.x + composition_screen_rect.right - composition_screen_rect.left,
                   pt.y + composition_screen_rect.bottom - composition_screen_rect.top};
  ResizeWebView();
}

LRESULT CompositionWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  using MSGHandler = LRESULT (CompositionWindow::*)(UINT, WPARAM, LPARAM, bool&);
  using MSGHandlerEntry = std::pair<UINT, MSGHandler>;
  static const MSGHandlerEntry s_msg_handlers[] = {
      {WM_DESTROY, &CompositionWindow::OnDestroy},
      {WM_CAPTURECHANGED, &CompositionWindow::OnCaptureChanged},
      {WM_SETCURSOR, &CompositionWindow::OnSetCursorChanged},
      {WM_MOVE, &CompositionWindow::OnMove},
      {WM_SIZE, &CompositionWindow::OnSized},
  };  // s_msg_handlers

  auto iter = std::find_if(std::begin(s_msg_handlers), std::end(s_msg_handlers),
                           [msg](const MSGHandlerEntry& e) { return e.first == msg; });
  if (iter != std::end(s_msg_handlers)) {
    bool handled = false;
    auto result = (this->*(iter->second))(msg, wParam, lParam, handled);
    if (handled) {
      return result;
    }
  }

  if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) {
    bool handled = false;
    auto result = OnMouseMessage(msg, wParam, lParam, handled);
    if (handled) {
      return result;
    }
  }

  return original_window_proc_(hWnd, msg, wParam, lParam);
}

void CompositionWindow::CreateWebView() {
  auto hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  assert(SUCCEEDED(hr));
  auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
  auto user_data_path = nullptr;  // Use default path
  hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, nullptr, options.Get(),
      Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this](HRESULT result,
                                                                                  ICoreWebView2Environment* environment)
                                                                               -> HRESULT {
        assert(environment != nullptr);
        environment->QueryInterface(webview_env_.GetAddressOf());
        assert(webview_env_ != nullptr);

        webview_env_->CreateCoreWebView2CompositionController(
            hwnd_, Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
                       this, &CompositionWindow::OnCreateCoreWebView2ControllerCompleted)
                       .Get());

        return S_OK;
      }).Get());

  assert(SUCCEEDED(hr));
}

HRESULT CompositionWindow::OnCreateCoreWebView2ControllerCompleted(
    HRESULT result,
    ICoreWebView2CompositionController* compositionController) {
  assert(compositionController != nullptr);

  webview_composition_controller_ = compositionController;
  auto hr = webview_composition_controller_.As(&webview_controller_);
  assert(SUCCEEDED(hr));

  hr = webview_composition_controller_->add_CursorChanged(
      Callback<ICoreWebView2CursorChangedEventHandler>([this](ICoreWebView2CompositionController* sender,
                                                              IUnknown* args) -> HRESULT {
        HCURSOR cursor = nullptr;
        auto hr = sender->get_Cursor(&cursor);
        assert(SUCCEEDED(hr));
        ::SetCursor(cursor);
        ::SetClassLongPtr(hwnd_, GCLP_HCURSOR, (LONG_PTR)cursor);
        return S_OK;
      }).Get(),
      &webview_cursor_changed_event_token_);
  assert(SUCCEEDED(hr));

  hr = webview_controller_->get_CoreWebView2(&webview_);
  assert(SUCCEEDED(hr));
  hr = webview_controller_->put_IsVisible(true);
  assert(SUCCEEDED(hr));
  hr = webview_controller_->put_DefaultBackgroundColor({0, 0, 0, 0});  // Transparent background
  assert(SUCCEEDED(hr));

  AddJSInterop(true /*before navigate*/);
  hr = webview_->Navigate(url_.c_str());
  assert(SUCCEEDED(hr));
  AddJSInterop(false /*after navigate*/);

  hr = ::DCompositionCreateDevice(nullptr, _uuidof(IDCompositionDevice), (void**)dcomp_device_.GetAddressOf());
  assert(SUCCEEDED(hr));

  hr = dcomp_device_->CreateTargetForHwnd(hwnd_, true, dcomp_target_.GetAddressOf());
  assert(SUCCEEDED(hr));

  hr = dcomp_device_->CreateVisual(&dcomp_visual_);
  assert(SUCCEEDED(hr));
  hr = dcomp_target_->SetRoot(dcomp_visual_.Get());
  assert(SUCCEEDED(hr));

  hr = webview_composition_controller_->put_RootVisualTarget(dcomp_visual_.Get());
  assert(SUCCEEDED(hr));

  ResizeWebView();
  return S_OK;
}

// 这里添加JS与Native互操作代码，演示支持的三种方式
void CompositionWindow::AddJSInterop(bool before_navigate) {
  HRESULT hr = S_OK;

  if (before_navigate) {
    // 方式一：注入一段JS到Web页面, 注意：必须在`webview_->Navigate`调用之前，否则无效
    // 注意：下面注入代码用到了通过`AddHostObjectToScript`注入的对象`sampleObject`，
    // 注入的对象会挂接在`chrome.webview.hostObjects`对象上。
    const wchar_t* js_code =
        L"function postNativeMessage() {\n"
        L"  chrome.webview.postMessage({name: 'hello', arg: 30});\n"
        L"  chrome.webview.addEventListener('message', event => console.log(event.data));\n"
        L"}\n"
        L"async function nativeAdd(...args) {\n"
        L"  let result = await chrome.webview.hostObjects.sampleObject.add(...args);\n"
        L"  return result;\n"
        L"}\n"
        L"async function nativeSub(...args) {\n"
        L"  let result = await chrome.webview.hostObjects.sampleObject.sub(...args);\n"
        L"  return result;\n"
        L"}\n"
        L"function syncNativeAdd(...args) {\n"
        L"  let result = chrome.webview.hostObjects.sync.sampleObject.add(...args);\n"
        L"  return result;\n"
        L"}\n"
        L"function syncNativeSub(...args) {\n"
        L"  let result = chrome.webview.hostObjects.sync.sampleObject.sub(...args);\n"
        L"  return result;\n"
        L"}\n";
    hr = webview_->AddScriptToExecuteOnDocumentCreated(
        js_code,
        Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>([](HRESULT error, PCWSTR id) {
          assert(SUCCEEDED(error));
          return S_OK;
        }).Get());
    assert(SUCCEEDED(hr));

    // 方式二：双向发送消息，任何时候都生效
    // 在web端通过如下方法发送及接收消息:
    // chrome.webview.postMessage(<obj>);
    // chrome.webview.addEventListener("message", (event) => console.log(event.data));
    hr = webview_->add_WebMessageReceived(
        Callback<ICoreWebView2WebMessageReceivedEventHandler>([this](ICoreWebView2* sender,
                                                                     ICoreWebView2WebMessageReceivedEventArgs* args) {
          HRESULT hr = S_OK;
          LPWSTR source = nullptr;
          LPWSTR string_message = nullptr;
          LPWSTR json_message = nullptr;

          hr = args->get_Source(&source);
          assert(SUCCEEDED(hr));
          hr = args->get_WebMessageAsJson(&json_message);
          if (FAILED(hr)) {
            hr = args->TryGetWebMessageAsString(&string_message);
            assert(SUCCEEDED(hr));
          }

          std::wstringstream ss;
          ss << L"Source: " << source << L"\n" << L"Content: " << (json_message ? json_message : string_message);
          ::MessageBoxW(hwnd_, ss.str().c_str(), L"WebMessage Received!", MB_ICONINFORMATION);

          // echo回去
          if (json_message) {
            webview_->PostWebMessageAsJson(json_message);
          } else {
            webview_->PostWebMessageAsString(string_message);
          }

          ::CoTaskMemFree(source);
          ::CoTaskMemFree(string_message);
          ::CoTaskMemFree(json_message);

          return S_OK;
        }).Get(),
        &webview_message_event_token_);
    assert(SUCCEEDED(hr));
  } else {
    // 方式三：添加Native对象供JS调用，注意：必须在`webview_->Navigate`调用之后，否则无效
    ComPtr<JSCallableObject> native_object = Make<JSCallableObject>();
    VARIANT var;
    var.vt = VT_DISPATCH;
    var.pdispVal = native_object.Detach();
    hr = webview_->AddHostObjectToScript(L"sampleObject", &var);
    assert(SUCCEEDED(hr));
  }
}

void CompositionWindow::ResizeWebView() {
  if (!webview_controller_) {
    return;
  }

  if (webview_rect_.left == webview_rect_.right || webview_rect_.top == webview_rect_.bottom) {
    webview_controller_->put_IsVisible(false);
  } else {
    dcomp_visual_->SetOffsetX(webview_rect_.left);
    dcomp_visual_->SetOffsetY(webview_rect_.top);
    auto hr = dcomp_device_->Commit();
    assert(SUCCEEDED(hr));

    webview_controller_->put_IsVisible(true);
    hr = webview_controller_->put_Bounds(webview_rect_);
    assert(SUCCEEDED(hr));
  }
}

void CompositionWindow::Cleanup() {
  if (hwnd_) {
    ::SetWindowLongPtrW(hwnd_, GWLP_USERDATA, NULL);
    ::SetWindowLongPtrW(hwnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_window_proc_));
    hwnd_ = nullptr;
  }

  if (webview_composition_controller_) {
    webview_composition_controller_->remove_CursorChanged(webview_cursor_changed_event_token_);
    webview_->remove_WebMessageReceived(webview_message_event_token_);
    webview_controller_->Close();
    webview_.Reset();
    webview_controller_.Reset();
    webview_composition_controller_.Reset();
  }

  dcomp_target_.Reset();
  dcomp_device_.Reset();
  dcomp_visual_.Reset();
}

LRESULT CompositionWindow::OnMove(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (webview_controller_) {
    webview_controller_->NotifyParentWindowPositionChanged();
  }
  return 0;
}

LRESULT CompositionWindow::OnSized(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (use_client_area_rect_) {
    ::GetClientRect(hwnd_, &webview_rect_);
    ResizeWebView();
  }
  return 0;
}

LRESULT CompositionWindow::OnCaptureChanged(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  mouse_capturing_ = false;
  return 0;
}

LRESULT CompositionWindow::OnSetCursorChanged(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  // 在网页区域要保留光标
  POINT pt;
  ::GetCursorPos(&pt);
  ::ScreenToClient(hwnd_, &pt);
  if (::PtInRect(&webview_rect_, pt)) {
    handled = true;
    return ::DefWindowProcW(hwnd_, msg, wParam, lParam);
  }

  return 0;
}

LRESULT CompositionWindow::OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  Cleanup();
  return 0;
}

LRESULT CompositionWindow::OnMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (!webview_composition_controller_) {
    return 0;
  }

  POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
  if (msg == WM_MOUSEWHEEL) {
    ::ScreenToClient(hwnd_, &pt);
  }

  UINT mouse_data = 0;

  // 不是自己track触发的`WM_MOUSELEAVE`消息要交给宿主窗口处理
  if (msg == WM_MOUSELEAVE && !mouse_tracking_) {
    return 0;
  }

  // 自己没有捕获鼠标时，不在自己区域的鼠标消息以及在自己区域但宿主窗口捕获了鼠标下都要丢给宿主窗口处理
  if ((!::PtInRect(&webview_rect_, pt) || ::GetCapture() == hwnd_) && !mouse_capturing_) {
    return 0;
  }

  if (WM_MOUSEWHEEL == msg) {
    mouse_data = GET_WHEEL_DELTA_WPARAM(wParam);
  } else if (WM_MOUSELEAVE == msg) {
    if (mouse_tracking_) {
      // Stop tracking mouse leave.
      TRACKMOUSEEVENT tme;
      tme.cbSize = sizeof(TRACKMOUSEEVENT);
      tme.dwFlags = TME_LEAVE & TME_CANCEL;
      tme.hwndTrack = hwnd_;
      ::TrackMouseEvent(&tme);
      mouse_tracking_ = false;
    }
  } else if (WM_XBUTTONDBLCLK == msg || WM_XBUTTONDOWN == msg || WM_XBUTTONUP == msg) {
    mouse_data = GET_XBUTTON_WPARAM(wParam);
  } else if (WM_LBUTTONDOWN == msg || WM_RBUTTONDOWN == msg || WM_XBUTTONDOWN == msg) {
    mouse_capturing_ = true;
    ::SetCapture(hwnd_);
  } else if (WM_LBUTTONUP == msg || WM_RBUTTONUP == msg || WM_XBUTTONUP == msg) {
    if (::GetCapture() == hwnd_) {
      mouse_capturing_ = false;
      ::ReleaseCapture();
    }
  } else if (WM_MOUSEMOVE == msg) {
    if (!mouse_tracking_) {
      TRACKMOUSEEVENT tme;
      tme.cbSize = sizeof(TRACKMOUSEEVENT);
      tme.dwFlags = TME_LEAVE;
      tme.hwndTrack = hwnd_;
      ::TrackMouseEvent(&tme);
      mouse_tracking_ = true;
    }
  }

  // 要转换到相对网页区域的坐标
  pt.x -= webview_rect_.left;
  pt.y -= webview_rect_.top;

  handled = true;
  auto hr = webview_composition_controller_->SendMouseInput(
      static_cast<COREWEBVIEW2_MOUSE_EVENT_KIND>(msg),
      static_cast<COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS>(GET_KEYSTATE_WPARAM(wParam)), mouse_data, pt);

  assert(SUCCEEDED(hr));

  return 0;
}