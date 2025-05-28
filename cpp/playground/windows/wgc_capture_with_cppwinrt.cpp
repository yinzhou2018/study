#include "wgc_capture_with_cppwinrt.h"
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <chrono>
#include <iostream>

using namespace winrt;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
  HMONITOR* pPrimaryMonitor = reinterpret_cast<HMONITOR*>(dwData);
  MONITORINFO monitorInfo;
  monitorInfo.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(hMonitor, &monitorInfo);

  if (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) {
    *pPrimaryMonitor = hMonitor;
    return FALSE;
  }
  return TRUE;
}

static HMONITOR GetPrimaryMonitor() {
  HMONITOR primaryMonitor = nullptr;
  EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&primaryMonitor));
  return primaryMonitor;
}

bool WGCCaptureWithCppWinRT::Initialize(HWND hwnd) {
  if (initialized_) {
    return true;
  }

  // 创建D3D11设备
  D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

  UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
  flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL selectedLevel;
  auto hr =
      D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, ARRAYSIZE(featureLevels),
                        D3D11_SDK_VERSION, d3dDevice_.put(), &selectedLevel, d3dContext_.put());
  if (FAILED(hr)) {
    std::cerr << "D3D11CreateDevice Failed: " << std::hex << hr << std::endl;
    return false;
  }

  auto dxgiDevice = d3dDevice_.as<IDXGIDevice>();
  com_ptr<IInspectable> inspectableDevice;
  hr = CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), inspectableDevice.put());
  if (FAILED(hr)) {
    std::cerr << "CreateDirect3D11DeviceFromDXGIDevice Failed: " << std::hex << hr << std::endl;
    return false;
  }
  device_ =
      IDirect3DDevice(inspectableDevice.as<ABI::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>().detach(),
                      take_ownership_from_abi);
  auto interop = get_activation_factory<GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
  if (!interop) {
    std::cerr << "Get GraphicsCaptureItemInterop Failed." << std::endl;
    return false;
  }
  if (hwnd) {
    hr = interop->CreateForWindow(hwnd, guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
                                  reinterpret_cast<void**>(put_abi(captureItem_)));
    if (FAILED(hr)) {
      std::cerr << "CreateForWindow Failed: " << std::hex << hr << std::endl;
      return false;
    }
  } else {
    hr =
        interop->CreateForMonitor(GetPrimaryMonitor(), guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
                                  reinterpret_cast<void**>(put_abi(captureItem_)));

    if (FAILED(hr)) {
      std::cerr << "CreateForMonitor Failed: " << std::hex << hr << std::endl;
      return false;
    }
  }

  auto size = captureItem_.Size();
  framePool_ =
      Direct3D11CaptureFramePool::CreateFreeThreaded(device_, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, size);

  // 创建捕获会话
  session_ = framePool_.CreateCaptureSession(captureItem_);
  session_.StartCapture();

  initialized_ = true;
  return true;
}

std::vector<uint8_t> WGCCaptureWithCppWinRT::Capture(int* pWidth, int* pHeight, UINT timeoutMs) {
  if (!initialized_) {
    return {};
  }

  try {
    auto startTime = std::chrono::steady_clock::now();

    while (true) {
      auto frame = framePool_.TryGetNextFrame();
      if (frame) {
        std::cout << "Frame ready time: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime)
                         .count()
                  << " ms" << std::endl;

        int width, height;
        auto pixels = GetPixelsFromTexture(d3dDevice_.get(), d3dContext_.get(), frame, width, height);

        if (pWidth)
          *pWidth = width;
        if (pHeight)
          *pHeight = height;
        return pixels;
      }

      if (timeoutMs != INFINITE) {
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > timeoutMs) {
          std::cout << "Timeout" << std::endl;
          return {};
        }
      }
    }
  } catch (winrt::hresult_error const& ex) {
    std::wcerr << L"捕获错误: " << ex.message().c_str() << std::endl;
    return {};
  }
}

std::vector<uint8_t> WGCCaptureWithCppWinRT::GetPixelsFromTexture(ID3D11Device* device,
                                                                  ID3D11DeviceContext* context,
                                                                  const Direct3D11CaptureFrame& frame,
                                                                  int& width,
                                                                  int& height) {
  try {
    auto surface = frame.Surface();
    auto interop = surface.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
    com_ptr<ID3D11Texture2D> texture;
    check_hresult(interop->GetInterface(guid_of<ID3D11Texture2D>(), texture.put_void()));

    D3D11_TEXTURE2D_DESC desc = {};
    texture->GetDesc(&desc);
    width = desc.Width;
    height = desc.Height;

    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.MiscFlags = 0;

    com_ptr<ID3D11Texture2D> stagingTexture;
    check_hresult(device->CreateTexture2D(&stagingDesc, nullptr, stagingTexture.put()));

    context->CopyResource(stagingTexture.get(), texture.get());

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    check_hresult(context->Map(stagingTexture.get(), 0, D3D11_MAP_READ, 0, &mapped));

    std::vector<uint8_t> pixels(width * height * 4);
    const uint8_t* src = static_cast<const uint8_t*>(mapped.pData);
    for (UINT row = 0; row < height; row++) {
      memcpy(&pixels[row * width * 4], src + row * mapped.RowPitch, width * 4);
    }

    context->Unmap(stagingTexture.get(), 0);
    return pixels;
  } catch (winrt::hresult_error const& ex) {
    std::wcerr << L"Get pixels failed: " << ex.message().c_str() << std::endl;
    return {};
  }
}
