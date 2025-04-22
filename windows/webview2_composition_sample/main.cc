#include "composition_window.h"

#include <memory>

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
                    _In_opt_ HINSTANCE hPrevInstance,
                    _In_ LPWSTR pszCmdLine,
                    _In_ int iCmdShow) {
  std::unique_ptr<CompositionWindow> window(CompositionWindow::Create(pszCmdLine));
  window->Show();

  MSG msg;
  while (::GetMessage(&msg, nullptr, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }

  return 0;
}