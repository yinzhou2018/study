#ifndef CEF_CEFOSR_OSR_APP_H_
#define CEF_CEFOSR_OSR_APP_H_

#include "include/cef_app.h"

class OsrWindow;
struct OsrRendererSettings;

// Implement application-level callbacks for the browser process.
class OsrApp : public CefApp, public CefBrowserProcessHandler, CefRenderProcessHandler {
 public:
  OsrApp(bool single_thread_mode) : single_thread_mode_(single_thread_mode) {}

  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return this; }

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;

  // CefRenderProcessHandler methods:
  void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefDOMNode> node) override;

  void CreateOsrWindow(const std::string& url, const OsrRendererSettings& settings);

 private:
  CefRefPtr<OsrWindow> window_ = nullptr;
  std::string url_;
  bool single_thread_mode_ = false;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(OsrApp);
};

#endif  // CEF_CEFOSR_OSR_APP_H_
