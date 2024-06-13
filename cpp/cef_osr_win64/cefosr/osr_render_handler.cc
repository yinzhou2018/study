#include "osr_render_handler.h"
#include "osr_utils.h"

#include "include/base/cef_callback.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

OsrRenderHandler::OsrRenderHandler(const OsrRendererSettings& settings, HWND hwnd)
    : settings_(settings),
      hwnd_(hwnd),

      weak_factory_(this) {
  CEF_REQUIRE_UI_THREAD();
  DCHECK(hwnd_);
}

OsrRenderHandler::~OsrRenderHandler() {
  CEF_REQUIRE_UI_THREAD();
}

void OsrRenderHandler::SetBrowser(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  browser_ = browser;
  if (browser_ && settings_.external_begin_frame_enabled) {
    // Start the BeginFrame timer.
    Invalidate();
  }
}

void OsrRenderHandler::Invalidate() {
  CEF_REQUIRE_UI_THREAD();
  if (begin_frame_pending_) {
    // The timer is already running.
    return;
  }

  // Trigger the BeginFrame timer.
  CHECK_GT(settings_.begin_frame_rate, 0);
  const float delay_us = (1.0 / double(settings_.begin_frame_rate)) * 1000000.0;
  TriggerBeginFrame(0, delay_us);
}

void OsrRenderHandler::TriggerBeginFrame(uint64_t last_time_us, float delay_us) {
  if (begin_frame_pending_ && !settings_.external_begin_frame_enabled) {
    // Render immediately and then wait for the next call to Invalidate() or
    // On[Accelerated]Paint().
    begin_frame_pending_ = false;
    Render();
    return;
  }

  const auto now = osr_utils::GetTimeNow();
  float offset = now - last_time_us;
  if (offset > delay_us) {
    offset = delay_us;
  }

  if (!begin_frame_pending_) {
    begin_frame_pending_ = true;
  }

  // Trigger again after the necessary delay to maintain the desired frame rate.
  CefPostDelayedTask(TID_UI,
                     base::BindOnce(&OsrRenderHandler::TriggerBeginFrame, weak_factory_.GetWeakPtr(), now, delay_us),
                     static_cast<int64_t>(offset / 1000.0));

  if (settings_.external_begin_frame_enabled && browser_) {
    // We're running the BeginFrame timer. Trigger rendering via
    // On[Accelerated]Paint().
    browser_->GetHost()->SendExternalBeginFrame();
  }
}