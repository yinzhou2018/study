#ifndef COMPOSITION_WINDOW_H_
#define COMPOSITION_WINDOW_H_
#include <WebView2.h>
#include <Windows.h>
#include <dcomp.h>
#include <wrl/client.h>

#include <string>

using namespace Microsoft::WRL;

class CompositionWindow {
 public:
  static CompositionWindow* Create(const wchar_t* url);
  void Show();

  private:
  CompositionWindow() = default;

  LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnSized(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnClose(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnMove(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);

  static LRESULT CALLBACK s_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  void CreateWebView();
  HRESULT OnCreateCoreWebView2ControllerCompleted(HRESULT result,
                                                  ICoreWebView2CompositionController* compositionController);
  void AddJSInterop(bool before_navigate);
  void ResizeWebView();

  bool mouse_tracking_ = false;
  HWND hwnd_ = nullptr;
  std::wstring url_;
  EventRegistrationToken webview_cursor_changed_event_token_;
  EventRegistrationToken webview_message_event_token_;
  ComPtr<ICoreWebView2> webview_;
  ComPtr<ICoreWebView2Controller2> webview_controller_;
  ComPtr<ICoreWebView2CompositionController> webview_composition_controller_;
  ComPtr<ICoreWebView2Environment3> webview_env_;
  ComPtr<IDCompositionDevice> dcomp_device_;
  ComPtr<IDCompositionTarget> dcomp_target_;
};  // CompositionWindow

#endif  // COMPOSITION_WINDOW_H_