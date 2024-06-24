#ifndef CEF_CEFOSR_OSR_RENDER_HANDLER_D3D11_H_
#define CEF_CEFOSR_OSR_RENDER_HANDLER_D3D11_H_
#pragma once

#include "osr_d3d11.h"
#include "osr_render_handler.h"
#include "osr_renderer_settings.h"

#include <dcomp.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class BrowserLayer : public d3d11::Layer {
 public:
  explicit BrowserLayer(const std::shared_ptr<d3d11::Device>& device);

  void render(const std::shared_ptr<d3d11::Context>& ctx) override;

  void on_paint(void* share_handle);
  void on_paint(void* buffer, int width, int height);

  // After calling on_paint() we can query the texture size.
  std::pair<uint32_t, uint32_t> texture_size() const;

 private:
  std::shared_ptr<d3d11::FrameBuffer> frame_buffer_;

  DISALLOW_COPY_AND_ASSIGN(BrowserLayer);
};

class PopupLayer : public BrowserLayer {
 public:
  explicit PopupLayer(const std::shared_ptr<d3d11::Device>& device);

  void set_bounds(const CefRect& bounds);

  bool contains(int x, int y) const { return bounds_.Contains(x, y); }
  int xoffset() const { return original_bounds_.x - bounds_.x; }
  int yoffset() const { return original_bounds_.y - bounds_.y; }

 private:
  CefRect original_bounds_;
  CefRect bounds_;

  DISALLOW_COPY_AND_ASSIGN(PopupLayer);
};

class OsrRenderHandlerD3D11 : public OsrRenderHandler {
 public:
  OsrRenderHandlerD3D11(const OsrRendererSettings& settings, HWND hwnd);

  bool Initialize(int width, int height);

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

  CefRenderHandler::RectList dirty_rects_;

  uint64_t start_time_ = 0;
  std::shared_ptr<d3d11::Device> device_;
  std::shared_ptr<d3d11::SwapChain> swap_chain_;
  std::shared_ptr<d3d11::Composition> composition_;
  std::shared_ptr<BrowserLayer> browser_layer_;
  std::shared_ptr<PopupLayer> popup_layer_;

  ComPtr<IDCompositionDevice> dcomp_device_;
  ComPtr<IDCompositionTarget> dcomp_target_;
  ComPtr<IDCompositionVisual> dcomp_visual_;

  DISALLOW_COPY_AND_ASSIGN(OsrRenderHandlerD3D11);
};

#endif  // CEF_CEFOSR_OSR_RENDER_HANDLER_D3D11_H_
