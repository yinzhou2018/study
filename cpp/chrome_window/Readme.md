#### win平台实现窗口背景透明几种方法：
- 传统的基于`Layered Window`实现，能实现按像素点击穿透，性能最差
- 基于DWM提供的相关接口实现，下面有要点说明
- 基于`WS_EX_NOREDIRECTIONBITMAP`窗口风格+[DirectComposition API](https://learn.microsoft.com/en-us/windows/win32/directcomp/architecture-and-components)提供的窗口合成层接口实现
#### 基于DWM实现窗口背景透明要点
- 创建窗口时去掉所有非客户区（边框，标题栏等），可以使用`WS_POPUP`风格，如果要实现点击穿透，窗口风格需带上`WS_EX_TRANSPARENT|WS_EX_LAYERED`；
- 在`WM_CREATE`消息响应将frame区域完全覆盖到client area：
```cpp
  // Negative margins have special meaning to DwmExtendFrameIntoClientArea.
  // Negative margins create the "sheet of glass" effect, where the client area
  // is rendered as a solid surface with no window border.
  MARGINS margins = {-1};
  HRESULT hr = S_OK;

  // Extend the frame across the entire window.
   DwmExtendFrameIntoClientArea(hwnd, &margins);
```
- 基于D2D绑定绘制窗口创建支持alpha通道的Renderer Target:
```cpp
    hr = pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                     D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
        D2D1::HwndRenderTargetProperties(hwnd, size), &pRenderTarget);
```
- 处理 `WM_PAINT`消息，基于D2D进行alpha透明背景绘制（注：GDI绘制失效）：
```cpp
  pRenderTarget->BeginDraw();
  pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f)); // 透明背景

  // TODO: 绘制具体UI元素...

  HRESULT hr = pRenderTarget->EndDraw();
```
- 响应 `WM_ERASEBKGND`消息，不要绘制背景，否则会在启动有一次白底闪烁：
#### 实现类似Chrome浏览器窗口风格要点
- 以`WS_OVERLAPPEDWINDOW`风格创建窗口
- 响应 `WM_NCCALCSIZE`消息调整客户区大小，覆盖标题栏区域：
```cpp
  if (FALSE == wParam) {
    // wParam为FALSE时，lParam指向一个RECT结构体
    // 这里我们不做任何修改，直接返回默认处理结果
    return DefWindowProc(hwnd, WM_NCCALCSIZE, wParam, lParam);
  } else {
    // 调用默认处理以获取默认的非客户区大小
    // LRESULT lResult = DefWindowProc(hwnd, WM_NCCALCSIZE, wParam, lParam);

    // lParam指向一个NCCALCSIZE_PARAMS结构体
    NCCALCSIZE_PARAMS* pNCCalcSize = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

    // 1. pNCCalcSize->rgrc[0].top是窗口上边缘的位置，保持不变完全覆盖标题栏
    // 2. 保留适当的边框区域方便拖拽调整窗口大小
    pNCCalcSize->rgrc[0].left += 3;
    pNCCalcSize->rgrc[0].right -= 4;
    pNCCalcSize->rgrc[0].bottom -= 3;

    // 返回0表示我们修改了客户区的大小
    return 0;
  }
```
- 响应`WM_NCHITTEST`消息，模拟非客户区位置点击，享用系统默认行为：
```cpp
  auto hr = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
  if (HTCLIENT == hr) {
    RECT rcWnd = {0};
    GetWindowRect(hwnd, &rcWnd);
    POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    if (pt.y <= rcWnd.top + 5) {
      hr = HTTOP;
    } else if (pt.y <= rcWnd.top + 60) {
      hr = HTCAPTION;
    }
  }

  return hr;
```