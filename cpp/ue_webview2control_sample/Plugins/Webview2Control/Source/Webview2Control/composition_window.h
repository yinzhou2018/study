#ifndef COMPOSITION_WINDOW_H_
#define COMPOSITION_WINDOW_H_
#include <WebView2.h>
#include <wrl/client.h>
#include <string>

// DirectComposition仅支持win8及以上系统，但UE支持win7，将一些系统宏定义写死到win7版本，
// 所以这里需要做些重定义以支持DirectComposition的依赖构建。
#if NTDDI_VERSION < NTDDI_WIN8
#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WIN8
#endif  // NTDDI_VERSION
#include <dcomp.h>

using namespace Microsoft::WRL;

class CompositionWindow {
 public:
  CompositionWindow() = default;
  void Initialize(HWND target_window, const RECT& composition_screen_rect, const std::wstring& url);
  void Initialize(HWND target_window, const std::wstring& url);
  void Shutdown();
  void ReiszeCompositionRect(const RECT& composition_screen_rect);

 private:
  LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnCaptureChanged(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnSetCursorChanged(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnMove(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnSized(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);

  static LRESULT CALLBACK s_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  void InitializeImpl(HWND target_window, const std::wstring& url);
  void CreateWebView();
  HRESULT OnCreateCoreWebView2ControllerCompleted(HRESULT result,
                                                  ICoreWebView2CompositionController* compositionController);
  void AddJSInterop(bool before_navigate);
  void ResizeWebView();
  void Cleanup();

  bool use_client_area_rect_ = false;
  bool mouse_tracking_ = false;
  bool mouse_capturing_ = false;
  HWND hwnd_ = nullptr;
  RECT webview_rect_ = {0};
  std::wstring url_;
  EventRegistrationToken webview_cursor_changed_event_token_;
  EventRegistrationToken webview_message_event_token_;
  ComPtr<ICoreWebView2> webview_;
  ComPtr<ICoreWebView2Controller2> webview_controller_;
  ComPtr<ICoreWebView2CompositionController> webview_composition_controller_;
  ComPtr<ICoreWebView2Environment3> webview_env_;
  ComPtr<IDCompositionDevice> dcomp_device_;
  ComPtr<IDCompositionTarget> dcomp_target_;
  ComPtr<IDCompositionVisual> dcomp_visual_;

  using WINDOW_PROC = LRESULT(CALLBACK*)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
  WINDOW_PROC original_window_proc_ = nullptr;
};  // CompositionWindow

#endif  // COMPOSITION_WINDOW_H_