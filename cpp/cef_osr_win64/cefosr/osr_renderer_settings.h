#ifndef CEF_CEFOSR_OSR_RENDERER_SETTINGS_H_
#define CEF_CEFOSR_OSR_RENDERER_SETTINGS_H_
#pragma once

#include "include/internal/cef_types.h"

enum class RHIType {
  RT_D3D11,
  RT_OPENGL,
};  // RHIType

struct OsrRendererSettings {
  OsrRendererSettings() = default;

  // If true draw a border around update rectangles.
  bool show_update_rect = false;

  // Background color. Enables transparency if the alpha component is 0.
  cef_color_t background_color = 0;

  // Render using shared textures. Supported on Windows only via D3D11.
  bool shared_texture_enabled = false;

  // Composition with native window rendering.
  bool composition_enabled = false;

  int frame_rate = 60;

  int log_render_cost_threshold = 16;
  int log_render_interval_threshold = 30;

  RHIType rhi_type = RHIType::RT_D3D11;

  // Client implements a BeginFrame timer by calling
  // CefBrowserHost::SendExternalBeginFrame at the specified frame rate.
  bool external_begin_frame_enabled = false;
  int begin_frame_rate = 0;
};

#endif  // CEF_CEFOSR_OSR_RENDERER_SETTINGS_H_
