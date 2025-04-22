// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class Webview2ControlSampleTarget : TargetRules
{
  public Webview2ControlSampleTarget(TargetInfo Target) : base(Target)
  {
    Type = TargetType.Game;
    DefaultBuildSettings = BuildSettingsVersion.V2;
    IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
    ExtraModuleNames.Add("Webview2ControlSample");
  }
}
