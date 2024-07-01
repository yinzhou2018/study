#include "composition_window.h"
#include "js_callable_object.h"

#include <WebView2EnvironmentOptions.h>
#include <windowsx.h>
#include <wrl/event.h>
#include <cassert>
#include <sstream>

using namespace Microsoft::WRL::Details;

LRESULT CALLBACK CompositionWindow::s_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  static CompositionWindow* window = nullptr;
  if (WM_NCCREATE == msg) {
    CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
    window = reinterpret_cast<CompositionWindow*>(cs->lpCreateParams);
    window->hwnd_ = hwnd;
  }
  return window ? window->WindowProc(hwnd, msg, wParam, lParam) : ::DefWindowProc(hwnd, msg, wParam, lParam);
}

CompositionWindow* CompositionWindow::Create(const wchar_t* url) {
  WNDCLASSEXW wcex = {0};
  LPCWSTR window_class = L"Webview2CompositionClass";
  LPCWSTR window_title = L"Webview2CompositionSample";

  wcex.cbSize = sizeof(wcex);
  wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
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
  ::RegisterClassExW(&wcex);

  auto window = new CompositionWindow();
  window->url_ = url;
  if (window->url_.empty()) {
    window->url_ = L"https://www.google.com";
  }
  ::CreateWindowExW(0, window_class, window_title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                    CW_USEDEFAULT, nullptr, nullptr, nullptr, window);
  return window;
}

void CompositionWindow::Show() {
  ::ShowWindow(hwnd_, SW_SHOW);
}

LRESULT CompositionWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  using MSGHandler = LRESULT (CompositionWindow::*)(UINT, WPARAM, LPARAM, bool&);
  using MSGHandlerEntry = std::pair<UINT, MSGHandler>;
  static const MSGHandlerEntry s_msg_handlers[] = {
      {WM_CREATE, &CompositionWindow::OnCreate},      {WM_SIZE, &CompositionWindow::OnSized},
      {WM_PAINT, &CompositionWindow::OnPaint},        {WM_CLOSE, &CompositionWindow::OnClose},
      {WM_MOVE, &CompositionWindow::OnMove},          {WM_DESTROY, &CompositionWindow::OnDestroy},
      {WM_TOUCH, &CompositionWindow::OnMouseMessage},
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

  return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CompositionWindow::OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  CreateWebView();
  return 0;
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

  ComPtr<IDCompositionVisual> dcomp_root_visual;
  hr = dcomp_device_->CreateVisual(&dcomp_root_visual);
  assert(SUCCEEDED(hr));
  hr = dcomp_target_->SetRoot(dcomp_root_visual.Get());
  assert(SUCCEEDED(hr));

  hr = webview_composition_controller_->put_RootVisualTarget(dcomp_root_visual.Get());
  assert(SUCCEEDED(hr));

  hr = dcomp_device_->Commit();
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
  RECT rc;
  ::GetClientRect(hwnd_, &rc);
  auto hr = webview_controller_->put_Bounds(rc);
  assert(SUCCEEDED(hr));
}

LRESULT CompositionWindow::OnSized(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (webview_controller_) {
    if (SIZE_MINIMIZED != wParam) {
      ResizeWebView();
      webview_controller_->put_IsVisible(true);
    } else {
      webview_controller_->put_IsVisible(false);
    }
  }

  return 0;
}

LRESULT CompositionWindow::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
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

  handled = true;
  return 0;
}

LRESULT CompositionWindow::OnClose(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  webview_composition_controller_->remove_CursorChanged(webview_cursor_changed_event_token_);
  webview_->remove_WebMessageReceived(webview_message_event_token_);
  webview_controller_->Close();
  webview_.Reset();
  webview_controller_.Reset();
  webview_composition_controller_.Reset();
  dcomp_target_.Reset();
  dcomp_device_.Reset();
  return 0;
}

LRESULT CompositionWindow::OnMove(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (webview_controller_) {
    webview_controller_->NotifyParentWindowPositionChanged();
  }
  return 0;
}

LRESULT CompositionWindow::OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  ::PostQuitMessage(0);
  return 0;
}
LRESULT CompositionWindow::OnMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (!webview_composition_controller_) {
    return 0;
  }

  POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
  UINT mouse_data = 0;

  if (WM_MOUSEWHEEL == msg) {
    mouse_data = GET_WHEEL_DELTA_WPARAM(wParam);
    ::ScreenToClient(hwnd_, &pt);
  } else if (WM_XBUTTONDBLCLK == msg || WM_XBUTTONDOWN == msg || WM_XBUTTONUP == msg) {
    mouse_data = GET_XBUTTON_WPARAM(wParam);
  } else if (WM_LBUTTONDOWN == msg || WM_RBUTTONDOWN == msg || WM_XBUTTONDOWN == msg) {
    ::SetCapture(hwnd_);
  } else if (WM_LBUTTONUP == msg || WM_RBUTTONUP == msg || WM_XBUTTONUP == msg) {
    if (::GetCapture() == hwnd_) {
      ::ReleaseCapture();
    }
  } else if (WM_MOUSEMOVE == msg) {
    if (!mouse_tracking_) {
      // Start tracking mouse leave. Required for the WM_MOUSELEAVE event to be generated.
      TRACKMOUSEEVENT tme;
      tme.cbSize = sizeof(TRACKMOUSEEVENT);
      tme.dwFlags = TME_LEAVE;
      tme.hwndTrack = hwnd_;
      ::TrackMouseEvent(&tme);
      mouse_tracking_ = true;
    }
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
  }

  auto hr = webview_composition_controller_->SendMouseInput(
      static_cast<COREWEBVIEW2_MOUSE_EVENT_KIND>(msg),
      static_cast<COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS>(GET_KEYSTATE_WPARAM(wParam)), mouse_data, pt);

  assert(SUCCEEDED(hr));

  return 0;
}