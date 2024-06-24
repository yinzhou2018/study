// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "include/wrapper/cef_helpers.h"

#include "osr_app.h"
#include "osr_renderer_settings.h"
#include "osr_window.h"

#include <thread>

void OsrApp::CreateOsrWindow(const std::string& url, const OsrRendererSettings& settings) {
  // const char* kUrl = "https://33tool.com/marquee/";
  const char* kUrl = "E:\\demo\\index.html";
  url_ = url.empty() ? kUrl : url;
  window_ = OsrWindow::Create(settings);

  if (single_thread_mode_) {
    window_->CreateBrowser(url_);
  }

  window_->Show();
}

void OsrApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  // 单线程情况下，会先进行Cef初始化，再创建窗口，初始化期间就会回调到这里，此时window还不存在，
  // 多线程情况下，独立线程进行Cef初始化，window已经提前创建出来，需在在这里创建Browser
  if (!single_thread_mode_) {
    window_->CreateBrowser(url_);
  }
}

void OsrApp::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefDOMNode> node) {
  static bool last_focused_is_editable = false;
  if (node && node->IsEditable() && !last_focused_is_editable) {
    last_focused_is_editable = true;
    auto message = CefProcessMessage::Create(kLastFocusedIsEditableMessage);
    frame->SendProcessMessage(PID_BROWSER, message);
  } else if ((!node || !node->IsEditable()) && last_focused_is_editable) {
    last_focused_is_editable = false;
    auto message = CefProcessMessage::Create(kLastFocusedIsNotEditableMessage);
    frame->SendProcessMessage(PID_BROWSER, message);
  }
}

int InitializeAndRunCEF(const CefMainArgs& main_args,
                        const CefSettings& settings,
                        OsrApp* app,
                        void* sandbox_info,
                        std::string* url,
                        OsrRendererSettings* renderer_settings) {
  // Initialize the CEF browser process. May return false if initialization
  // fails or if early exit is desired (for example, due to process singleton
  // relaunch behavior).
  if (!CefInitialize(main_args, settings, app, sandbox_info)) {
    return CefGetExitCode();
  }

  // 单线程情况下，必须在Cef初始化后创建窗口，否则有UI显示不正常
  if (url && renderer_settings) {
    app->CreateOsrWindow(*url, *renderer_settings);
  }

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();
  return 0;
}

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.
// Uncomment this line to manually enable sandbox support.
// #define CEF_USE_SANDBOX 1

#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library may not link successfully with all VS
// versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif

// Entry point function for all processes.
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  int exit_code;

  // Parse command-line arguments for use in this method.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromString(::GetCommandLineW());

  auto single_thread_mode = command_line->HasSwitch("single-thread-mode");
  // auto single_thread_mode = true;

#if defined(ARCH_CPU_32_BITS)
  // Run the main thread on 32-bit Windows using a fiber with the preferred 4MiB
  // stack size. This function must be called at the top of the executable entry
  // point function (`main()` or `wWinMain()`). It is used in combination with
  // the initial stack size of 0.5MiB configured via the `/STACK:0x80000` linker
  // flag on executable targets. This saves significant memory on threads (like
  // those in the Windows thread pool, and others) whose stack size can only be
  // controlled via the linker flag.
  exit_code = CefRunWinMainWithPreferredStackSize(wWinMain, hInstance, lpCmdLine, nCmdShow);
  if (exit_code >= 0) {
    // The fiber has completed so return here.
    return exit_code;
  }
#endif

  void* sandbox_info = nullptr;

#if defined(CEF_USE_SANDBOX)
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  // Provide CEF with command-line arguments.
  CefMainArgs main_args(hInstance);

  // SimpleApp implements application-level callbacks for the browser process.
  // It will create the first browser instance in OnContextInitialized() after
  // CEF has initialized.
  CefRefPtr<OsrApp> app(new OsrApp(single_thread_mode));

  // CEF applications have multiple sub-processes (render, GPU, etc) that share
  // the same executable. This function checks the command-line and, if this is
  // a sub-process, executes the appropriate logic.
  exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

  // Specify CEF global settings here.
  CefSettings settings;

  // Use the CEF Chrome bootstrap unless "--disable-chrome-runtime" is specified
  // via the command-line. Otherwise, use the CEF Alloy bootstrap. The Alloy
  // bootstrap is deprecated and will be removed in ~M127. See
  // https://github.com/chromiumembedded/cef/issues/3685
  settings.chrome_runtime = !command_line->HasSwitch("disable-chrome-runtime");

#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif

  std::string url = command_line->GetSwitchValue("url");
  OsrRendererSettings renderer_settings;
  renderer_settings.frame_rate = command_line->GetSwitchValue("frame-rate").empty()
                                     ? 60
                                     : atoi(std::string(command_line->GetSwitchValue("frame-rate")).c_str());
  renderer_settings.shared_texture_enabled = command_line->HasSwitch("shared-texture-enabled");
  renderer_settings.composition_enabled = !command_line->HasSwitch("composition-disabled");

  if (single_thread_mode) {
    InitializeAndRunCEF(main_args, settings, app.get(), sandbox_info, &url, &renderer_settings);
  } else {
    app->CreateOsrWindow(url, renderer_settings);
    auto cef_thread = std::thread(&InitializeAndRunCEF, main_args, settings, app.get(), sandbox_info, nullptr, nullptr);
    MSG msg = {0};
    while (::GetMessage(&msg, nullptr, 0, 0)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }
    cef_thread.join();
  }

  return 0;
}