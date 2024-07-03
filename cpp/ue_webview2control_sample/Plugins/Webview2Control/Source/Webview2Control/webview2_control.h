
#pragma once

#include <memory>
#include "CoreMinimal.h"

class CompositionWindow;
class FWebview2ControlModule : public IModuleInterface {
 public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

 private:
  void OnBeginPlay(bool simulating);
  void OnEndPlay(bool simulating);
  void CreateCompositionWindow();
  void DestroyCompositionWindow();
  void OnViewportResized(FViewport* Viewport, uint32);

  std::unique_ptr<CompositionWindow> composition_window_;
};