#ifndef CEF_CEFOSR_OSR_WINDOW_H_
#define CEF_CEFOSR_OSR_WINDOW_H_

#include "include/cef_client.h"

class OsrImeHandler;
class OsrInputHandler;
class OsrRenderHandler;
struct OsrRendererSettings;

constexpr const char* kLastFocusedIsEditableMessage = "last_focused_is_editable";
constexpr const char* kLastFocusedIsNotEditableMessage = "last_focused_is_not_editable";

class OsrWindow : public CefClient,
                  CefLifeSpanHandler,
                  CefRenderHandler,
                  CefLoadHandler,
                  CefDisplayHandler,
                  CefContextMenuHandler {
 public:
  static CefRefPtr<OsrWindow> Create(const std::string& url, const OsrRendererSettings& settings);
  void Show();

 private:
  OsrWindow() = default;

  // CefClient methods:
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override { return this; }
  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override;

  // CefLifeSpanHandler methods:
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler methods:
  void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;

  // CefDisplayHandler methods:
  bool OnCursorChange(CefRefPtr<CefBrowser> browser,
                      CefCursorHandle cursor,
                      cef_cursor_type_t type,
                      const CefCursorInfo& custom_cursor_info) override;
  bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) override;

  // CefRenderHandler methods:
  bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;
  void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
  bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY) override;
  void OnPaint(CefRefPtr<CefBrowser> browser,
               PaintElementType type,
               const RectList& dirtyRects,
               const void* buffer,
               int width,
               int height) override;
  void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                          PaintElementType type,
                          const RectList& dirtyRects,
                          const CefAcceleratedPaintInfo& info) override;
  void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
  void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;
  void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,
                                    const CefRange& selected_range,
                                    const RectList& character_bounds) override;

  LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  LRESULT OnSized(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnNativePaint(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnEraseBackground(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnDPIChanged(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnClose(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnMove(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnFocusMessage(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);

  // CefContextMenuHandler methods:
  void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefContextMenuParams> params,
                           CefRefPtr<CefMenuModel> model) override;

  static LRESULT CALLBACK s_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK s_getmessage_hook_proc(int code, WPARAM wParam, LPARAM lParam);

  CefRefPtr<CefBrowser> browser_ = nullptr;
  bool browser_hidden_ = false;
  float scale_factor_ = 0.0f;
  HWND hwnd_ = nullptr;
  HHOOK hook_ = nullptr;

  std::unique_ptr<OsrImeHandler> ime_handler_;
  std::unique_ptr<OsrInputHandler> input_handler_;
  std::unique_ptr<OsrRenderHandler> render_handler_;
  friend class OsrImeHandler;
  friend class OsrInputHandler;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(OsrWindow);
};  // OsrWindow

#endif  // CEF_CEFOSR_OSR_WINDOW_H_