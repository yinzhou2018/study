#include "osr_render_handler_d3d11.h"
#include "osr_utils.h"

#include "include/base/cef_callback.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#pragma comment(lib, "Dcomp.lib")

BrowserLayer::BrowserLayer(const std::shared_ptr<d3d11::Device>& device) : d3d11::Layer(device, false /* flip */) {
  frame_buffer_ = std::make_shared<d3d11::FrameBuffer>(device_);
}

void BrowserLayer::render(const std::shared_ptr<d3d11::Context>& ctx) {
  // Use the base class method to draw our texture.
  render_texture(ctx, frame_buffer_->texture());
}

void BrowserLayer::on_paint(void* share_handle) {
  frame_buffer_->on_paint(share_handle);
}

void BrowserLayer::on_paint(void* buffer, int width, int height) {
  frame_buffer_->on_paint(buffer, width, height);
}

std::pair<uint32_t, uint32_t> BrowserLayer::texture_size() const {
  const auto texture = frame_buffer_->texture();
  return std::make_pair(texture->width(), texture->height());
}

PopupLayer::PopupLayer(const std::shared_ptr<d3d11::Device>& device) : BrowserLayer(device) {}

void PopupLayer::set_bounds(const CefRect& bounds) {
  const auto comp = composition();
  if (!comp) {
    return;
  }

  const auto outer_width = comp->width();
  const auto outer_height = comp->height();
  if (outer_width == 0 || outer_height == 0) {
    return;
  }

  original_bounds_ = bounds;
  bounds_ = bounds;

  // If x or y are negative, move them to 0.
  if (bounds_.x < 0) {
    bounds_.x = 0;
  }
  if (bounds_.y < 0) {
    bounds_.y = 0;
  }
  // If popup goes outside the view, try to reposition origin
  if (bounds_.x + bounds_.width > outer_width) {
    bounds_.x = outer_width - bounds_.width;
  }
  if (bounds_.y + bounds_.height > outer_height) {
    bounds_.y = outer_height - bounds_.height;
  }
  // If x or y became negative, move them to 0 again.
  if (bounds_.x < 0) {
    bounds_.x = 0;
  }
  if (bounds_.y < 0) {
    bounds_.y = 0;
  }

  const auto x = bounds_.x / float(outer_width);
  const auto y = bounds_.y / float(outer_height);
  const auto w = bounds_.width / float(outer_width);
  const auto h = bounds_.height / float(outer_height);
  move(x, y, w, h);
}

OsrRenderHandlerD3D11::OsrRenderHandlerD3D11(const OsrRendererSettings& settings, HWND hwnd)
    : OsrRenderHandler(settings, hwnd) {
  RECT rc;
  ::GetClientRect(hwnd, &rc);
  Initialize(rc.right, rc.bottom);
}

bool OsrRenderHandlerD3D11::Initialize(int width, int height) {
  // Create a D3D11 device instance.
  device_ = d3d11::Device::create();
  DCHECK(device_);
  if (!device_) {
    return false;
  }

  // Create a D3D11 swapchain for the window.
  swap_chain_ = device_->create_swapchain(hwnd(), composition_enabled());
  DCHECK(swap_chain_);
  if (!swap_chain_) {
    return false;
  }

  if (composition_enabled()) {
    auto device_if = (ID3D11Device*)*device_;
    ComPtr<IDXGIDevice> dxgi_device;
    device_if->QueryInterface((IDXGIDevice**)&dxgi_device);
    DCompositionCreateDevice(dxgi_device.Get(), __uuidof(IDCompositionDevice), &dcomp_device_);
    dcomp_device_->CreateTargetForHwnd(hwnd(), TRUE, &dcomp_target_);
    dcomp_device_->CreateVisual(&dcomp_visual_);
    dcomp_visual_->SetContent((IDXGISwapChain1*)*swap_chain_);
    dcomp_target_->SetRoot(dcomp_visual_.Get());
  }

  // Create the browser layer.
  browser_layer_ = std::make_shared<BrowserLayer>(device_);

  // Set up the composition.
  composition_ = std::make_shared<d3d11::Composition>(device_, width, height);
  composition_->add_layer(browser_layer_);

  // Size to the whole composition.
  browser_layer_->move(0.0f, 0.0f, 1.0f, 1.0f);

  start_time_ = osr_utils::GetTimeNow();

  return true;
}

bool OsrRenderHandlerD3D11::IsOverPopupWidget(int x, int y) const {
  return popup_layer_ && popup_layer_->contains(x, y);
}

int OsrRenderHandlerD3D11::GetPopupXOffset() const {
  if (popup_layer_) {
    return popup_layer_->xoffset();
  }
  return 0;
}

int OsrRenderHandlerD3D11::GetPopupYOffset() const {
  if (popup_layer_) {
    return popup_layer_->yoffset();
  }
  return 0;
}

void OsrRenderHandlerD3D11::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) {
  if (show) {
    DCHECK(!popup_layer_);

    // Create a new layer.
    popup_layer_ = std::make_shared<PopupLayer>(device_);
    composition_->add_layer(popup_layer_);
  } else {
    DCHECK(popup_layer_);

    composition_->remove_layer(popup_layer_);
    popup_layer_ = nullptr;

    Render();
  }
}

void OsrRenderHandlerD3D11::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) {
  popup_layer_->set_bounds(rect);
}

void OsrRenderHandlerD3D11::OnPaint(CefRefPtr<CefBrowser> browser,
                                    CefRenderHandler::PaintElementType type,
                                    const CefRenderHandler::RectList& dirtyRects,
                                    const void* buffer,
                                    int width,
                                    int height) {
  dirty_rects_ = dirtyRects;
  if (type == PET_POPUP) {
    popup_layer_->on_paint((void*)buffer, width, height);
  } else {
    browser_layer_->on_paint((void*)buffer, width, height);
  }

  Render();
}

void OsrRenderHandlerD3D11::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                               CefRenderHandler::PaintElementType type,
                                               const CefRenderHandler::RectList& dirtyRects,
                                               const CefAcceleratedPaintInfo& info) {
  dirty_rects_ = dirtyRects;
  if (type == PET_POPUP) {
    popup_layer_->on_paint(info.shared_texture_handle);
  } else {
    browser_layer_->on_paint(info.shared_texture_handle);
  }

  Render();
}

void OsrRenderHandlerD3D11::Render() {
  // Update composition + layers based on time.
  const auto t = (osr_utils::GetTimeNow() - start_time_) / 1000000.0;
  composition_->tick(t);

  auto ctx = device_->immedidate_context();
  swap_chain_->bind(ctx);

  const auto texture_size = browser_layer_->texture_size();

  // Resize the composition and swap chain to match the texture if necessary.
  composition_->resize(!send_begin_frame(), texture_size.first, texture_size.second);
  swap_chain_->resize(texture_size.first, texture_size.second);

  // Clear the render target.
  if (composition_enabled()) {
    swap_chain_->clear(0.0f, 0.0f, 0.0f, 0.0f);
  } else {
    swap_chain_->clear(1.0f, 1.0f, 1.0f, 1.0f);
  }

  // Render the scene.
  composition_->render(ctx);

  // Present to window.
  swap_chain_->present(1);

  if (composition_enabled()) {
    dcomp_device_->Commit();
  }
}