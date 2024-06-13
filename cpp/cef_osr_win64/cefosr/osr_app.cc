// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "include/wrapper/cef_helpers.h"

#include "osr_app.h"
#include "osr_window.h"

OsrApp::OsrApp() = default;

void OsrApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();
  // const char* kUrl = "https://www.google.com";
  const char* kUrl = "E:\\demo.html";
  window_ = OsrWindow::Create(kUrl);
  window_->Show();
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
  CefRefPtr<OsrApp> app(new OsrApp);

  // CEF applications have multiple sub-processes (render, GPU, etc) that share
  // the same executable. This function checks the command-line and, if this is
  // a sub-process, executes the appropriate logic.
  exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

  // Parse command-line arguments for use in this method.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromString(::GetCommandLineW());

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

  // Initialize the CEF browser process. May return false if initialization
  // fails or if early exit is desired (for example, due to process singleton
  // relaunch behavior).
  if (!CefInitialize(main_args, settings, app.get(), sandbox_info)) {
    return CefGetExitCode();
  }

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}