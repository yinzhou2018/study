#include "screen_capture.h"
#include "dxgi_capture.h"

#include <errhandlingapi.h>
#include <windef.h>
#include <windows.h>
#include <winuser.h>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

struct PerformanceCounter {
  PerformanceCounter(const char* name) : name_(name) { start_ = std::chrono::high_resolution_clock::now(); }
  ~PerformanceCounter() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
    std::cout << name_ << " spend: " << duration.count() << " ms" << std::endl;
  }

  std::chrono::high_resolution_clock::time_point start_;
  const char* name_;
};  // PerformanceCounter

static void GetWindowDimensions(HWND hwnd, int& width, int& height) {
  if (!hwnd) {
    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);
  } else {
    RECT rect;
    GetWindowRect(hwnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
  }
}

static std::vector<uint8_t> CaptureRegion(HDC hdcSource, int width, int height) {
  // 创建兼容DC和位图
  HDC hdcMemory = CreateCompatibleDC(hdcSource);
  HBITMAP hBitmap = CreateCompatibleBitmap(hdcSource, width, height);
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemory, hBitmap);

  // 执行位图拷贝（这是一个高性能操作）
  BitBlt(hdcMemory, 0, 0, width, height, hdcSource, 0, 0, SRCCOPY);

  // 准备位图信息
  BITMAPINFOHEADER bi;
  ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = width;
  bi.biHeight = -height;  // 负值表示自上而下的位图
  bi.biPlanes = 1;
  bi.biBitCount = 32;  // RGBA
  bi.biCompression = BI_RGB;

  // 分配内存存储像素数据
  std::vector<uint8_t> buffer(width * height * 4);  // 32位 = 4字节每像素

  // 获取位图数据
  GetDIBits(hdcMemory, hBitmap, 0, height, buffer.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

  // 清理资源
  SelectObject(hdcMemory, hOldBitmap);
  DeleteObject(hBitmap);
  DeleteDC(hdcMemory);

  return buffer;
}

static std::vector<uint8_t> CaptureWindowWithGDI(HWND hwnd) {
  PerformanceCounter counter(hwnd ? "CaptureWindowWithGDI-Window" : "CaptureScreenWithGDI-Screen");
  HDC hdcWindow = GetWindowDC(hwnd);
  if (!hdcWindow)
    return {};
  int width, height;
  GetWindowDimensions(hwnd, width, height);
  auto result = CaptureRegion(hdcWindow, width, height);
  ReleaseDC(hwnd, hdcWindow);
  return result;
}

static std::vector<uint8_t> CaptureWindowWithPrintWindow(HWND hwnd) {
  PerformanceCounter counter(hwnd ? "CaptureWindowWithPrintWindow-Window" : "CaptureScreenWithPrintWindow-Screen");

  int width, height;
  GetWindowDimensions(hwnd, width, height);

  // 创建兼容DC和位图
  HDC hdcScreen = GetDC(NULL);
  HDC hdcMemory = CreateCompatibleDC(hdcScreen);
  HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemory, hBitmap);

  // 使用PrintWindow捕获窗口
  // PW_RENDERFULLCONTENT标志用于捕获DirectX等内容，win8.1开始才支持
  auto hr = PrintWindow(hwnd, hdcMemory, PW_RENDERFULLCONTENT);
  if (!hr) {
    std::cerr << "PrintWindow failed: " << GetLastError() << std::endl;
    SelectObject(hdcMemory, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMemory);
    ReleaseDC(NULL, hdcScreen);
    return {};
  }

  // 准备位图信息
  BITMAPINFOHEADER bi;
  ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = width;
  bi.biHeight = -height;  // 负值表示自上而下的位图
  bi.biPlanes = 1;
  bi.biBitCount = 32;  // RGBA
  bi.biCompression = BI_RGB;

  // 分配内存存储像素数据
  std::vector<uint8_t> buffer(width * height * 4);  // 32位 = 4字节每像素

  // 获取位图数据
  GetDIBits(hdcMemory, hBitmap, 0, height, buffer.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

  // 清理资源
  SelectObject(hdcMemory, hOldBitmap);
  DeleteObject(hBitmap);
  DeleteDC(hdcMemory);
  ReleaseDC(NULL, hdcScreen);

  return buffer;
}

// 保存截图为BMP文件
void SaveToBMP(const std::vector<uint8_t>& buffer, int width, int height, const char* filename) {
  if (buffer.empty())
    return;

  BITMAPFILEHEADER fileHeader = {0};
  BITMAPINFOHEADER infoHeader = {0};

  fileHeader.bfType = 0x4D42;  // "BM"
  fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + buffer.size();
  fileHeader.bfReserved1 = 0;
  fileHeader.bfReserved2 = 0;
  fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  infoHeader.biSize = sizeof(BITMAPINFOHEADER);
  infoHeader.biWidth = width;
  infoHeader.biHeight = -height;  // 负值表示自上而下的位图
  infoHeader.biPlanes = 1;
  infoHeader.biBitCount = 32;
  infoHeader.biCompression = BI_RGB;
  infoHeader.biSizeImage = buffer.size();
  infoHeader.biXPelsPerMeter = 0;
  infoHeader.biYPelsPerMeter = 0;
  infoHeader.biClrUsed = 0;
  infoHeader.biClrImportant = 0;

  std::ofstream file(filename, std::ios::binary);
  file.write(reinterpret_cast<char*>(&fileHeader), sizeof(BITMAPFILEHEADER));
  file.write(reinterpret_cast<char*>(&infoHeader), sizeof(BITMAPINFOHEADER));
  file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

// 对于不是用GDI绘制的窗口该方法无效，整个屏幕依然可以用GDI方式捕获
static void capture_win_with_gdi(HWND hwnd) {
  auto windowBuffer = CaptureWindowWithGDI(hwnd);
  if (!windowBuffer.empty()) {
    int width, height;
    GetWindowDimensions(hwnd, width, height);
    SaveToBMP(windowBuffer, width, height, hwnd ? "window_with_gdi.bmp" : "screen_with_gdi.bmp");
    if (hwnd) {
      std::cout << "Window captured and saved to window_with_gdi.bmp" << std::endl;
    } else {
      std::cout << "Screen captured and saved to screen.bmp" << std::endl;
    }
  }
}

static void capture_win_with_print_window(HWND hwnd) {
  auto windowBuffer = CaptureWindowWithPrintWindow(hwnd);
  if (!windowBuffer.empty()) {
    int width, height;
    GetWindowDimensions(hwnd, width, height);
    SaveToBMP(windowBuffer, width, height, hwnd ? "window_with_print.bmp" : "screen_with_print.bmp");
    if (hwnd) {
      std::cout << "Window captured and saved to window_with_print.bmp" << std::endl;
    } else {
      std::cout << "Screen captured and saved to screen_with_print.bmp" << std::endl;
    }
  }
}

static std::vector<uint8_t> CaptureWithDxgi(HWND hwnd, int* pWidth, int* pHeight) {
  static DxgiScreenCapture dxgiCapture;
  dxgiCapture.Initialize();
  PerformanceCounter counter(hwnd ? "CaptureWithDxgi-Window" : "CaptureWithDxgi-Screen");
  return dxgiCapture.Capture(hwnd, pWidth, pHeight);
}

static void capture_win_with_dxgi(HWND hwnd) {
  if (!DxgiScreenCapture::IsSupported()) {
    return;
  }

  int width, height;
  auto buffer = CaptureWithDxgi(hwnd, &width, &height);
  if (!buffer.empty()) {
    SaveToBMP(buffer, width, height, hwnd ? "window_with_dxgi.bmp" : "screen_with_dxgi.bmp");
    if (hwnd) {
      std::cout << "Window captured and saved to window_with_dxgi.bmp" << std::endl;
    } else {
      std::cout << "Screen captured and saved to screen_with_dxgi.bmp" << std::endl;
    }
  }
}

void win_screen_capture_test() {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  capture_win_with_gdi(nullptr);
  capture_win_with_dxgi(nullptr);

  // PrintWindow只能捕获普通窗口，不能捕获桌面
  // capture_win_with_print_window(nullptr);

  // 获取当前窗口并截图
  HWND activeWindow = GetForegroundWindow();
  std::cout << "activeWindow: " << activeWindow << std::endl;

  capture_win_with_gdi(activeWindow);
  capture_win_with_print_window(activeWindow);
  capture_win_with_dxgi(activeWindow);
}