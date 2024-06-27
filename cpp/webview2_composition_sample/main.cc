#include <WebView2.h>
#include <Windows.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

ComPtr<ICoreWebView2> s_webview;
ComPtr<ICoreWebView2Controller2> s_controller;
ComPtr<ICoreWebView2CompositionController> s_composition_controller;

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_DESTROY) {
    ::PostQuitMessage(0);
  }
  return ::DefWindowProc(hWnd, message, wParam, lParam);
}

HWND create_window() {
  WNDCLASSEXW wcex = {0};
  LPCWSTR window_class = L"Webview2CompositionClass";
  LPCWSTR window_title = L"Webview2CompositionSample";

  wcex.cbSize = sizeof(wcex);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = window_proc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = GetModuleHandle(nullptr);
  wcex.hIcon = nullptr;
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH));
  wcex.lpszMenuName = nullptr;
  wcex.lpszClassName = window_class;
  wcex.hIconSm = nullptr;
  ::RegisterClassExW(&wcex);

  return ::CreateWindowExW(0, window_class, window_title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                           CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
                    _In_opt_ HINSTANCE hPrevInstance,
                    _In_ LPWSTR pszCmdLine,
                    _In_ int iCmdShow) {
  auto hwnd = create_window();
  ::ShowWindow(hwnd, SW_SHOWNORMAL);

  MSG msg;
  while (::GetMessage(&msg, nullptr, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }

  return 0;
}