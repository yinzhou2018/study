#include "dxgi_capture.h"

#include <chrono>
#include <iostream>

bool DxgiScreenCapture::IsSupported() {
  // 创建临时D3D设备来检查DXGI 1.2支持
  Microsoft::WRL::ComPtr<ID3D11Device> device;
  D3D_FEATURE_LEVEL featureLevel;
  HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &device,
                                 &featureLevel, nullptr);

  if (FAILED(hr)) {
    return false;
  }

  Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
  hr = device.As(&dxgiDevice);
  if (FAILED(hr)) {
    return false;
  }

  Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
  hr = dxgiDevice->GetAdapter(&dxgiAdapter);
  if (FAILED(hr)) {
    return false;
  }

  Microsoft::WRL::ComPtr<IDXGIOutput> dxgiOutput;
  hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput);
  if (FAILED(hr)) {
    return false;
  }

  Microsoft::WRL::ComPtr<IDXGIOutput1> dxgiOutput1;
  hr = dxgiOutput.As(&dxgiOutput1);
  return SUCCEEDED(hr);
}

bool DxgiScreenCapture::Initialize(HWND hwnd) {
  if (!InitD3D11Device()) {
    return false;
  }
  if (!InitDuplication(hwnd, &duplication_)) {
    return false;
  }

  if (hwnd) {
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo = {sizeof(MONITORINFO)};
    GetMonitorInfo(monitor, &monitorInfo);
    int monitorLeft = monitorInfo.rcMonitor.left;
    int monitorTop = monitorInfo.rcMonitor.top;
    RECT rect;
    GetWindowRect(hwnd, &rect);
    auto x = rect.left - monitorLeft;
    auto y = rect.top - monitorTop;
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    if (x < 0) {
      width += x;
      x = 0;
    }
    if (y < 0) {
      height += y;
      y = 0;
    }
    copyRect_ = {x, y, x + width, y + height};
  }

  hwnd_ = hwnd;
  initialized_ = true;
  return true;
}

bool DxgiScreenCapture::InitD3D11Device() {
  // 清理现有设备
  device_.Reset();
  context_.Reset();

  UINT flags = 0;
#ifdef _DEBUG
  flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
                                       D3D_FEATURE_LEVEL_10_0};

  D3D_FEATURE_LEVEL featureLevel;
  HRESULT hr = D3D11CreateDevice(nullptr,                   // 使用默认适配器
                                 D3D_DRIVER_TYPE_HARDWARE,  // 使用硬件设备
                                 nullptr,                   // 不使用软件光栅器
                                 flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device_,
                                 &featureLevel, &context_);

  if (FAILED(hr)) {
    std::cerr << "Failed to create D3D11 device with hardware acceleration\n";

    // 如果硬件加速失败，尝试使用WARP设备
    hr = D3D11CreateDevice(nullptr,
                           D3D_DRIVER_TYPE_WARP,  // 使用WARP设备
                           nullptr, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device_,
                           &featureLevel, &context_);

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D11 device with WARP\n";
      return false;
    }

    std::cerr << "Created D3D11 device using WARP\n";
  }

  return true;
}

bool DxgiScreenCapture::InitDuplication(HWND hwnd, IDXGIOutputDuplication** duplication) {
  // 获取DXGI设备
  Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
  HRESULT hr = device_.As(&dxgiDevice);
  if (FAILED(hr)) {
    std::cerr << "Failed to get DXGI device\n";
    return false;
  }

  // 获取DXGI适配器
  Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
  hr = dxgiDevice->GetAdapter(&dxgiAdapter);
  if (FAILED(hr)) {
    std::cerr << "Failed to get DXGI adapter\n";
    return false;
  }

  UINT displayIndex = 0;
  Microsoft::WRL::ComPtr<IDXGIOutput> dxgiOutput;
  if (hwnd) {
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    while (dxgiAdapter->EnumOutputs(displayIndex, &dxgiOutput) != DXGI_ERROR_NOT_FOUND) {
      DXGI_OUTPUT_DESC outputDesc;
      if (SUCCEEDED(dxgiOutput->GetDesc(&outputDesc))) {
        if (outputDesc.Monitor == monitor) {
          break;
        }
      }
      displayIndex++;
      dxgiOutput.Reset();
    }
  } else {
    dxgiAdapter->EnumOutputs(displayIndex, &dxgiOutput);
  }

  if (!dxgiOutput) {
    std::cerr << "Failed to get DXGI output " << displayIndex << "\n";
    return false;
  }

  // 转换为DXGI Output 1接口
  Microsoft::WRL::ComPtr<IDXGIOutput1> dxgiOutput1;
  hr = dxgiOutput.As(&dxgiOutput1);
  if (FAILED(hr)) {
    std::cerr << "Failed to get IDXGIOutput1 interface\n";
    return false;
  }

  // 获取输出描述
  DXGI_OUTPUT_DESC outputDesc;
  hr = dxgiOutput1->GetDesc(&outputDesc);
  if (FAILED(hr)) {
    std::cerr << "Failed to get output description\n";
    return false;
  }

  // 创建Desktop Duplication
  hr = dxgiOutput1->DuplicateOutput(device_.Get(), duplication);
  if (FAILED(hr)) {
    return false;
  }

  return true;
}

std::vector<uint8_t> DxgiScreenCapture::Capture(int* pWidth, int* pHeight, UINT timeoutms) {
  DXGI_OUTDUPL_FRAME_INFO frameInfo;
  ZeroMemory(&frameInfo, sizeof(frameInfo));
  Microsoft::WRL::ComPtr<IDXGIResource> desktopResource;

  auto now = std::chrono::steady_clock::now();
START:
  HRESULT hr = duplication_->AcquireNextFrame(timeoutms, &frameInfo, &desktopResource);

  if (hr == DXGI_ERROR_ACCESS_LOST) {
    std::cout << "Access lost, reinitializing...\n";
    duplication_.Reset();
    if (!InitDuplication(hwnd_, &duplication_)) {
      std::cout << "Failed to reinitialize duplication\n";
      return {};
    }
    goto START;
  }

  if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
    std::cout << "Frame timeout, no new frame available\n";
    return {};
  }

  if (FAILED(hr)) {
    std::cout << "Failed to acquire frame: " << std::hex << hr << std::dec << "\n";
    return {};
  }

  // 很重要：屏幕数据并未变化，这种情况destkopResource为空，需要重新获取，但理论上有死循环的风险
  if (frameInfo.AccumulatedFrames == 0 || frameInfo.LastPresentTime.QuadPart == 0) {
    duplication_->ReleaseFrame();
    desktopResource = nullptr;
    goto START;
  }
  std::cout << "DXGI Ready time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - now).count()
            << "ms\n";

  // 获取桌面纹理
  Microsoft::WRL::ComPtr<ID3D11Texture2D> desktopTexture;
  hr = desktopResource.As(&desktopTexture);
  if (FAILED(hr)) {
    std::cout << "Failed to get desktop texture\n";
    return {};
  }

  // 获取桌面纹理描述
  D3D11_TEXTURE2D_DESC desktopDesc;
  desktopTexture->GetDesc(&desktopDesc);

  auto width = desktopDesc.Width;
  auto height = desktopDesc.Height;
  auto x = 0;
  auto y = 0;
  if (!IsRectEmpty(&copyRect_)) {
    x = copyRect_.left;
    y = copyRect_.top;
    width = copyRect_.right - copyRect_.left;
    height = copyRect_.bottom - copyRect_.top;
  }

  // 创建暂存纹理，确保格式匹配
  D3D11_TEXTURE2D_DESC stagingDesc = {};
  stagingDesc.Width = width;
  stagingDesc.Height = height;
  stagingDesc.MipLevels = 1;
  stagingDesc.ArraySize = 1;
  stagingDesc.Format = desktopDesc.Format;  // 使用相同的格式
  stagingDesc.SampleDesc.Count = 1;
  stagingDesc.SampleDesc.Quality = 0;
  stagingDesc.Usage = D3D11_USAGE_STAGING;
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  stagingDesc.BindFlags = 0;
  stagingDesc.MiscFlags = 0;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
  hr = device_->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
  if (FAILED(hr)) {
    std::cout << "Failed to create staging texture\n";
    return std::vector<uint8_t>();
  }

  // 复制到暂存纹理
  D3D11_BOX box = {(UINT)x, (UINT)y, 0, x + width, y + height, 1};
  context_->CopySubresourceRegion(stagingTexture.Get(), 0, 0, 0, 0, desktopTexture.Get(), 0, &box);

  // 映射暂存纹理以读取像素数据
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  hr = context_->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
  if (FAILED(hr)) {
    std::cout << "Failed to map staging texture\n";
    return std::vector<uint8_t>();
  }

  // 复制像素数据
  std::vector<uint8_t> frameData(width * height * 4);
  const uint8_t* src = static_cast<const uint8_t*>(mappedResource.pData);

  for (UINT row = 0; row < height; row++) {
    memcpy(&frameData[row * width * 4], src + row * mappedResource.RowPitch, width * 4);
  }

  // 解除映射并释放帧
  context_->Unmap(stagingTexture.Get(), 0);

  *pWidth = width;
  *pHeight = height;
  return frameData;
}
