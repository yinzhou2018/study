#ifndef CEF_CEFOSR_OSR_APP_H_
#define CEF_CEFOSR_OSR_APP_H_

#include "include/cef_app.h"

class OsrWindow;

// Implement application-level callbacks for the browser process.
class OsrApp : public CefApp, public CefBrowserProcessHandler, CefRenderProcessHandler {
 public:
  OsrApp();

  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return this; }

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;

  // CefRenderProcessHandler methods:
  void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefDOMNode> node) override;

 private:
  CefRefPtr<OsrWindow> window_ = nullptr;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(OsrApp);
};

#endif  // CEF_CEFOSR_OSR_APP_H_
