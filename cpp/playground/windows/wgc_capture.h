#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <roapi.h>

#include <windows.devices.display.h>
#include <windows.foundation.h>
#include <windows.graphics.capture.h>
#include <windows.graphics.directx.direct3d11.interop.h>

#include <wrl/event.h>
#include <wrl/implements.h>
#include <wrl/wrappers/corewrappers.h>

#include <chrono>
#include <iostream>
#include <vector>

using namespace Microsoft::WRL::Wrappers;
using namespace Microsoft::WRL;
using namespace ABI::Windows::Graphics::Capture;
using namespace ABI::Windows::Graphics::DirectX::Direct3D11;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Devices::Display;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Foundation;

class WGCCapture {
 public:
  bool Initialize(HWND hwnd);
  std::vector<uint8_t> Capture(int* pWidth, int* pHeight, UINT timeoutms = INFINITE);

 private:
  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<IDirect3DDevice> d3ddevice_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
  ComPtr<IDirect3D11CaptureFramePool> framePool_;
  ComPtr<IGraphicsCaptureSession> session_;
  bool initialized_{false};
};