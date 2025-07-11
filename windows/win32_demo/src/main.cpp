#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>
#include <iostream>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

// Direct2D 资源
ID2D1Factory* pFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
ID2D1SolidColorBrush* pBrush = nullptr;
IDWriteFactory* pDWriteFactory = nullptr;
IDWriteTextFormat* pTextFormat = nullptr;

// 创建 Direct2D 资源
HRESULT CreateD2DResources(HWND hwnd) {
  HRESULT hr = S_OK;

  if (!pRenderTarget) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
    auto properties = D2D1::RenderTargetProperties();
    auto dpi = GetDpiForWindow(hwnd);
    properties.dpiX = properties.dpiY = dpi;
    hr = pFactory->CreateHwndRenderTarget(properties, D2D1::HwndRenderTargetProperties(hwnd, size), &pRenderTarget);

    if (SUCCEEDED(hr)) {
      hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBrush);
    }
  }

  return hr;
}

void DiscardD2DResources() {
  if (pBrush)
    pBrush->Release();
  if (pRenderTarget)
    pRenderTarget->Release();
  pRenderTarget = nullptr;
  pBrush = nullptr;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  // 初始化 Direct2D
  HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
  if (FAILED(hr)) {
    std::cout << "Failed to create Direct2D factory." << std::endl;
    return 0;
  }

  // 创建 DirectWrite 工厂
  hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                           reinterpret_cast<IUnknown**>(&pDWriteFactory));
  if (FAILED(hr)) {
    std::cout << "Failed to create DirectWrite factory." << std::endl;
    return 0;
  }

  // 创建文本格式
  hr = pDWriteFactory->CreateTextFormat(L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
                                        DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"en-us", &pTextFormat);
  if (FAILED(hr)) {
    std::cout << "Failed to create text format." << std::endl;
    return 0;
  }

  // 设置文本对齐方式
  pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  // 注册窗口类
  const wchar_t CLASS_NAME[] = L"Win32 Demo Window";

  WNDCLASS wc = {};
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

  RegisterClass(&wc);

  // 创建窗口
  HWND hwnd = CreateWindowEx(0,                    // 扩展窗口样式
                             CLASS_NAME,           // 窗口类名
                             L"Win32 Demo",        // 窗口标题
                             WS_OVERLAPPEDWINDOW,  // 窗口样式

                             // 大小和位置
                             CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,

                             NULL,       // 父窗口
                             NULL,       // 菜单
                             hInstance,  // 实例句柄
                             NULL        // 附加数据
  );

  if (hwnd == NULL) {
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);

  RECT rc;
  GetWindowRect(hwnd, &rc);
  std::cout << "Window size: " << rc.right - rc.left << "x" << rc.bottom - rc.top << std::endl;

  // 消息循环
  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}

void DrawWithD2D(HWND hwnd) {
  HRESULT hr = CreateD2DResources(hwnd);
  if (FAILED(hr)) {
    return;
  }

  pRenderTarget->BeginDraw();
  pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

  D2D1_SIZE_F size = pRenderTarget->GetSize();
  D2D1_SIZE_U pixelSize = pRenderTarget->GetPixelSize();
  float dpiX, dpiY;
  pRenderTarget->GetDpi(&dpiX, &dpiY);
  std::cout << "Render target size: " << size.width << "x" << size.height << std::endl;
  std::cout << "Pixel size: " << pixelSize.width << "x" << pixelSize.height << std::endl;
  std::cout << "DPI: " << dpiX << "x" << dpiY << std::endl;

  float left = 10;
  float top = 10;
  float rectWidth = 100;
  float rectHeight = 100;

  D2D1_RECT_F rectangle = D2D1::RectF(left, top, left + rectWidth, top + rectHeight);

  // 绘制矩形边框
  pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
  pRenderTarget->DrawRectangle(rectangle, pBrush, 2.0f);

  // 在矩形中绘制文本
  pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
  pRenderTarget->DrawText(L"Hello World", 11, pTextFormat, rectangle, pBrush);

  left = size.width - 10 - rectWidth;
  top = size.height - 10 - rectHeight;
  rectangle = {left, top, left + rectWidth, top + rectHeight};
  pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
  pRenderTarget->DrawRectangle(rectangle, pBrush, 2.0f);
  pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
  pRenderTarget->DrawText(L"Hello World", 11, pTextFormat, rectangle, pBrush);

  hr = pRenderTarget->EndDraw();
  if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
    DiscardD2DResources();
  }
}

void DrawWithDC(HWND hwnd) {
  PAINTSTRUCT ps;
  BeginPaint(hwnd, &ps);
  HDC hdc = ps.hdc;

  RECT clientRect;
  GetClientRect(hwnd, &clientRect);
  int squareSize = 100;
  int x = (clientRect.right - clientRect.left - squareSize) / 2;
  int y = (clientRect.bottom - clientRect.top - squareSize) / 2;
  Rectangle(hdc, x, y, x + squareSize, y + squareSize);

  y += squareSize + 10;
  Rectangle(hdc, x, y, x + squareSize, y + squareSize);

  EndPaint(hwnd, &ps);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_DESTROY:
      DiscardD2DResources();
      PostQuitMessage(0);
      return 0;

    case WM_SIZE:
      if (pRenderTarget) {
        RECT rc;
        GetClientRect(hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        pRenderTarget->Resize(size);
      }
      return 0;

    case WM_PAINT: {
      // 1. D2D底层采用了双缓冲，绘制在后台缓冲进行，在调用EndDraw时提交最新的RedirectedSurface给DWM做合成
      // 2. 而DC绘制是直接在当前RedirectedSurface上绘制，存在一边绘制一边合成上屏的情况，导致闪烁或撕裂的问题
      // 3.
      // 如果先调用DrawWithDC，再调用DrawWithD2D，会导致DC绘制的内容被D2D绘制的内容覆盖，因为D2D是完整的提交新的RedirectedSurface
      // 4.
      // 如果需要D2D与DC混合绘制，可以创建GDI互操作的RenderTarget，然后再从RenderTarget获取DC，或者创建基于DC的RenderTarget
      DrawWithD2D(hwnd);
      DrawWithDC(hwnd);
      return 0;
    }
    case WM_DISPLAYCHANGE:
    case WM_DPICHANGED: {
      std::cout << "Display or dpi change detected." << std::endl;
      DiscardD2DResources();
      InvalidateRect(hwnd, NULL, FALSE);
      return 0;
    }
    case WM_ERASEBKGND:
      std::cout << "WM_ERASEBKGND" << std::endl;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
