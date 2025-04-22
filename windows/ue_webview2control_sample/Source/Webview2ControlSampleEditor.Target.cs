// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class Webview2ControlSampleEditorTarget : TargetRules
{
  public Webview2ControlSampleEditorTarget(TargetInfo Target) : base(Target)
  {
    Type = TargetType.Editor;
    DefaultBuildSettings = BuildSettingsVersion.V2;
    IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
    ExtraModuleNames.Add("Webview2ControlSample");
  }
}
