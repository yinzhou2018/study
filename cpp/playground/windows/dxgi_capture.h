#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <vector>

class DxgiScreenCapture {
 public:
  bool Initialize(HWND hwnd);

  std::vector<uint8_t> Capture(int* pWidth, int* pHeight, UINT timeoutms = INFINITE);

  // 检查系统是否支持DXGI 1.2
  static bool IsSupported();

 private:
  bool InitD3D11Device();
  bool InitDuplication(HWND hwnd, IDXGIOutputDuplication** duplication);

 private:
  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
  Microsoft::WRL::ComPtr<IDXGIOutputDuplication> duplication_;
  RECT copyRect_{0, 0, 0, 0};
  HWND hwnd_{nullptr};
  bool initialized_{false};
};