#include "wgc_capture.h"

ComPtr<IGraphicsCaptureItem> CreateCaptureItemForWindow(HWND hwnd) {
  ComPtr<IGraphicsCaptureItemStatics2> itemStatics;
  HRESULT hr = RoGetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureItem).Get(),
                                      IID_PPV_ARGS(&itemStatics));
  if (FAILED(hr)) {
    std::cout << "Failed to get IGraphicsCaptureItemStatics\n";
    return nullptr;
  }
  ABI::Windows::UI::WindowId windowId{(UINT64)hwnd};
  ComPtr<IGraphicsCaptureItem> item;
  hr = itemStatics->TryCreateFromWindowId(windowId, &item);
  if (FAILED(hr)) {
    std::cout << "Failed to create IGraphicsCaptureItem from window\n";
    return nullptr;
  }
  return item;
}

bool WGCCapture::Initialize(HWND hwnd) {
  if (initialized_) {
    return true;
  }

  D3D_FEATURE_LEVEL featureLevel;
  D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
  HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, ARRAYSIZE(featureLevels),
                                 D3D11_SDK_VERSION, &device_, &featureLevel, nullptr);
  if (FAILED(hr)) {
    std::cout << "Failed to create D3D11 device: " << hr << "\n";
    return false;
  }

  device_->GetImmediateContext(&context_);

  ComPtr<IDXGIDevice> dxgiDevice;
  device_.As(&dxgiDevice);
  ComPtr<IInspectable> inspectableDevice;
  ComPtr<IDirect3DDxgiInterfaceAccess> dxgiInterfaceAccess;
  hr = CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.Get(), &inspectableDevice);
  if (FAILED(hr)) {
    std::cout << "Failed to create IDirect3DDevice: " << hr << "\n";
    return false;
  }

  hr = inspectableDevice.As(&d3ddevice_);
  if (FAILED(hr)) {
    std::cout << "Failed to cast to IDirect3DDevice: " << hr << "\n";
    return false;
  }

  auto captureItem = CreateCaptureItemForWindow(hwnd);
  if (!captureItem) {
    std::cout << "Failed to create capture item\n";
    return false;
  }

  ComPtr<IDirect3D11CaptureFramePoolStatics> framePoolStatics;
  hr = RoGetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Capture_Direct3D11CaptureFramePool).Get(),
                              IID_PPV_ARGS(&framePoolStatics));
  if (FAILED(hr)) {
    std::cout << "Failed to get IGraphicsCaptureFramePoolStatics\n";
    return false;
  }

  ABI::Windows::Graphics::SizeInt32 size;
  captureItem->get_Size(&size);
  hr = framePoolStatics->Create(
      d3ddevice_.Get(), ABI::Windows::Graphics::DirectX::DirectXPixelFormat::DirectXPixelFormat_B8G8R8A8UIntNormalized,
      2, size, &framePool_);
  if (FAILED(hr)) {
    std::cout << "Failed to create frame pool\n";
    return false;
  }

  hr = framePool_->CreateCaptureSession(captureItem.Get(), &session_);
  if (FAILED(hr)) {
    std::cout << "Failed to create capture session\n";
    return false;
  }

  hr = session_->StartCapture();
  if (FAILED(hr)) {
    std::cout << "Failed to start capture\n";
    return false;
  }

  initialized_ = true;
  return true;
}

std::vector<uint8_t> GetPixels(ID3D11Device* device,
                               ID3D11DeviceContext* context,
                               IDirect3D11CaptureFrame* frame,
                               int& width,
                               int& height) {
  ComPtr<IDirect3DSurface> surfaceObj;
  auto hr = frame->get_Surface(&surfaceObj);
  if (FAILED(hr) || !surfaceObj) {
    std::cerr << "Failed to get surface: " << hr << std::endl;
    return {};
  }

  ComPtr<IDirect3DDxgiInterfaceAccess> dxgiAccess;
  hr = surfaceObj.As(&dxgiAccess);
  if (FAILED(hr)) {
    std::cerr << "Failed to get IDirect3DDxgiInterfaceAccess: " << hr << std::endl;
    return {};
  }

  ComPtr<ID3D11Texture2D> texture;
  hr = dxgiAccess->GetInterface(IID_PPV_ARGS(&texture));
  if (FAILED(hr)) {
    std::cerr << "Failed to get ID3D11Texture2D: " << hr << std::endl;
    return {};
  }

  D3D11_TEXTURE2D_DESC desc = {};
  texture->GetDesc(&desc);
  width = desc.Width;
  height = desc.Height;

  D3D11_TEXTURE2D_DESC stagingDesc = desc;
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  stagingDesc.Usage = D3D11_USAGE_STAGING;
  stagingDesc.BindFlags = 0;
  stagingDesc.MiscFlags = 0;

  ComPtr<ID3D11Texture2D> stagingTex;
  hr = device->CreateTexture2D(&stagingDesc, nullptr, &stagingTex);
  if (FAILED(hr)) {
    std::cerr << "Failed to create staging texture: " << hr << std::endl;
    return {};
  }

  context->CopyResource(stagingTex.Get(), texture.Get());
  D3D11_MAPPED_SUBRESOURCE mapped = {};
  hr = context->Map(stagingTex.Get(), 0, D3D11_MAP_READ, 0, &mapped);
  if (FAILED(hr)) {
    std::cerr << "Failed to map staging texture: " << hr << std::endl;
    return {};
  }

  std::vector<uint8_t> pixels(desc.Width * desc.Height * 4);
  const uint8_t* src = static_cast<const uint8_t*>(mapped.pData);
  for (UINT row = 0; row < height; row++) {
    memcpy(&pixels[row * width * 4], src + row * mapped.RowPitch, width * 4);
  }

  context->Unmap(stagingTex.Get(), 0);
  return pixels;
}

std::vector<uint8_t> WGCCapture::Capture(int* pWidth, int* pHeight, UINT timeoutms) {
  auto now = std::chrono::steady_clock::now();
  do {
    ComPtr<IDirect3D11CaptureFrame> frame;
    auto hr = framePool_->TryGetNextFrame(&frame);
    if (FAILED(hr)) {
      std::cout << "Failed to get next frame" << std::endl;
      return {};
    }
    if (!frame) {
      if (timeoutms != INFINITE && std::chrono::steady_clock::now() - now > std::chrono::milliseconds(timeoutms)) {
        std::cout << "Timeout waiting for frame" << std::endl;
        return {};
      }
      continue;
    }
    std::cout << "WGC Ready time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - now).count()
              << " ms" << std::endl;
    int width, height;
    auto pixels = GetPixels(device_.Get(), context_.Get(), frame.Get(), width, height);
    *pWidth = width;
    *pHeight = height;
    return pixels;
  } while (true);

  // RegisterFrameArrivedHandler(framePool.Get(), d3dDevice.Get());
  // while (!g_frameSaved) {
  //   MSG msg;
  //   if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
  //     TranslateMessage(&msg);
  //     DispatchMessage(&msg);
  //   } else {
  //     Sleep(100);  // 等待帧到达
  //   }
  // }
}

void RegisterFrameArrivedHandler(IDirect3D11CaptureFramePool* framePool, ID3D11Device* d3dDevice) {
  auto handler = Callback<ITypedEventHandler<Direct3D11CaptureFramePool*, IInspectable*>>(
      [d3dDevice](IDirect3D11CaptureFramePool* sender, IInspectable* args) -> HRESULT {
        ComPtr<IDirect3D11CaptureFrame> frame;
        HRESULT hr = sender->TryGetNextFrame(&frame);
        if (FAILED(hr) || !frame)
          return S_OK;

        ComPtr<ID3D11DeviceContext> context;
        d3dDevice->GetImmediateContext(&context);
        int width, height;
        auto pixels = GetPixels(d3dDevice, context.Get(), frame.Get(), width, height);
        return S_OK;
      });

  EventRegistrationToken token;
  auto hr = framePool->add_FrameArrived(handler.Get(), &token);
  if (FAILED(hr)) {
    std::cerr << "Failed to add frame arrived handler: " << hr << std::endl;
  }
}