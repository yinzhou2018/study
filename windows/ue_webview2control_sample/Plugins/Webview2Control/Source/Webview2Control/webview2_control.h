
#pragma once

#include <memory>
#include "CoreMinimal.h"

class CompositionWindow;
class FWebview2ControlModule : public IModuleInterface {
 public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

 private:
 #if WITH_EDITOR
  void OnBeginPlay(bool simulating);
  void OnEndPlay(bool simulating);
  void OnViewportResized(FViewport* Viewport, uint32);
#endif  // WITH_EDITOR

  void CreateCompositionWindow();
  void DestroyCompositionWindow();

  std::unique_ptr<CompositionWindow> composition_window_;
};