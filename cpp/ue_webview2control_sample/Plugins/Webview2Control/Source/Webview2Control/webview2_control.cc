#include "webview2_control.h"
#include "composition_window.h"

#if WITH_EDITOR
#include "Editor.h"
#endif  // WITH_EDITOR
#include "Widgets/SViewport.h"

IMPLEMENT_MODULE(FWebview2ControlModule, Webview2Control)

DECLARE_LOG_CATEGORY_EXTERN(Webview2Control, Log, All);
DEFINE_LOG_CATEGORY(Webview2Control);

RECT CalcGameViewportScreenRect(UGameViewportClient* viewport) {
  auto viewport_widget = viewport->GetGameViewportWidget();
  auto viewport_geometry = viewport_widget->GetCachedGeometry();
  auto position = viewport_geometry.GetAbsolutePosition();
  auto size = viewport->Viewport->GetSizeXY();
  return RECT{(LONG)position.X, (LONG)position.Y, (LONG)size.X + (LONG)position.X, (LONG)size.Y + (LONG)position.Y};
}

void FWebview2ControlModule::StartupModule() {
#if WITH_EDITOR
  FEditorDelegates::PostPIEStarted.AddRaw(this, &FWebview2ControlModule::OnBeginPlay);
  FEditorDelegates::EndPIE.AddRaw(this, &FWebview2ControlModule::OnEndPlay);
#endif  // WITH_EDITOR
}

void FWebview2ControlModule::ShutdownModule() {
#if WITH_EDITOR
  FEditorDelegates::BeginPIE.RemoveAll(this);
  FEditorDelegates::EndPIE.RemoveAll(this);
#endif  // WITH_EDITOR
}

void FWebview2ControlModule::OnBeginPlay(bool simulating) {
  CreateCompositionWindow();
}

void FWebview2ControlModule::OnEndPlay(bool simulating) {
  DestroyCompositionWindow();
}

void FWebview2ControlModule::CreateCompositionWindow() {
  auto viewport = GEngine->GameViewport;
  if (!viewport) {
    UE_LOG(Webview2Control, Error,
           TEXT("Failed to create composition window since GameViewport has not been created yet."));
    return;
  }

  auto window = viewport->GetWindow();
  if (!window.IsValid()) {
    UE_LOG(Webview2Control, Error,
           TEXT("Failed to create composition window since the GEngine->GameViewport->GetWindow() returns NULL."));
    return;
  }

  auto native_window = window->GetNativeWindow();
  if (!native_window.IsValid()) {
    UE_LOG(
        Webview2Control, Error,
        TEXT("Failed to create composition window since GameViewport->GetWindow()->GetNativeWindow() returns NULL."));
    return;
  }

  auto hwnd = (HWND)native_window->GetOSWindowHandle();
  if (!::IsWindow(hwnd)) {
    UE_LOG(Webview2Control, Error, TEXT("Failed to create composition window because the returned hwnd is invalid!"));
    return;
  }

  auto rect = CalcGameViewportScreenRect(GEngine->GameViewport);
  composition_window_.reset(new CompositionWindow());
  composition_window_->Initalize(hwnd, rect, L"https://www.baidu.com");

  FViewport::ViewportResizedEvent.AddRaw(this, &FWebview2ControlModule::OnViewportResized);
}

void FWebview2ControlModule::DestroyCompositionWindow() {
  composition_window_->Shutdown();
  composition_window_.reset();
  FViewport::ViewportResizedEvent.RemoveAll(this);
}

void FWebview2ControlModule::OnViewportResized(FViewport* viewport, uint32) {
  if (GEngine->GameViewport && GEngine->GameViewport->Viewport == viewport) {
    composition_window_->ReiszeCompositionRect(CalcGameViewportScreenRect(GEngine->GameViewport));
  }
}
