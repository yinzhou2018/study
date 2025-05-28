#pragma once

#include <d3d11.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <vector>

class WGCCaptureWithCppWinRT {
 public:
  bool Initialize(HWND hwnd);
  std::vector<uint8_t> Capture(int* pWidth, int* pHeight, UINT timeoutMs = 1000);

 private:
  bool initialized_ = false;

  winrt::com_ptr<ID3D11Device> d3dDevice_;
  winrt::com_ptr<ID3D11DeviceContext> d3dContext_;

  winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice device_{nullptr};
  winrt::Windows::Graphics::Capture::GraphicsCaptureItem captureItem_{nullptr};
  winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool framePool_{nullptr};
  winrt::Windows::Graphics::Capture::GraphicsCaptureSession session_{nullptr};

  static std::vector<uint8_t> GetPixelsFromTexture(
      ID3D11Device* device,
      ID3D11DeviceContext* context,
      const winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame& frame,
      int& width,
      int& height);
};
