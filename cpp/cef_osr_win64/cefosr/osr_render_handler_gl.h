#ifndef CEF_CEFOSR_OSR_RENDER_HANDLER_GL_H_
#define CEF_CEFOSR_OSR_RENDER_HANDLER_GL_H_
#pragma once

#include "osr_render_handler.h"
#include "osr_renderer.h"

class OsrRenderHandlerGL : public OsrRenderHandler {
 public:
  OsrRenderHandlerGL(const OsrRendererSettings& settings, HWND hwnd);
  ~OsrRenderHandlerGL() override;

  // Must be called immediately after object creation.
  void Initialize(CefRefPtr<CefBrowser> browser);

  bool IsOverPopupWidget(int x, int y) const override;
  int GetPopupXOffset() const override;
  int GetPopupYOffset() const override;
  void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
  void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;
  void OnPaint(CefRefPtr<CefBrowser> browser,
               CefRenderHandler::PaintElementType type,
               const CefRenderHandler::RectList& dirtyRects,
               const void* buffer,
               int width,
               int height) override;
  void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                          CefRenderHandler::PaintElementType type,
                          const CefRenderHandler::RectList& dirtyRects,
                          const CefAcceleratedPaintInfo& info) override;

 private:
  void Render() override;

  void EnableGL();
  void DisableGL();

  // The below members are only accessed on the UI thread.
  OsrRenderer renderer_;
  HDC hdc_ = nullptr;
  HGLRC hrc_ = nullptr;
  bool painting_popup_ = false;

  DISALLOW_COPY_AND_ASSIGN(OsrRenderHandlerGL);
};

#endif  // CEF_CEFOSR_OSR_RENDER_HANDLER_GL_H_
