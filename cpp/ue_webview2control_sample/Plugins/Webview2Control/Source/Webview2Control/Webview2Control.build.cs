using UnrealBuildTool;
using System.IO;

public class Webview2Control : ModuleRules
{
  public Webview2Control(ReadOnlyTargetRules Target) : base(Target)
  {
    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

    if (Target.bBuildEditor)
    {
      PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "LevelEditor", "Slate", "SlateCore" });
    }

    // 添加WebView2 SDK依赖
    if (Target.Platform == UnrealTargetPlatform.Win64)
    {
      string Webview2SDKPath = Path.Combine(ModuleDirectory, "Microsoft.Web.WebView2.1.0.2592.51/sdk/native");
      PublicIncludePaths.Add(Path.Combine(Webview2SDKPath, "include"));
      PublicAdditionalLibraries.Add(Path.Combine(Webview2SDKPath, "x64/WebView2Loader.dll.lib"));
      PublicAdditionalLibraries.Add("dcomp.lib");

      var LoaderDllPath = Path.Combine(Webview2SDKPath, "x64/WebView2Loader.dll");
      RuntimeDependencies.Add(LoaderDllPath);
      string BinariesDir = Path.Combine(ModuleDirectory, "../../Binaries/Win64");
      if (!Directory.Exists(BinariesDir))
      {
        Directory.CreateDirectory(BinariesDir);
      }
      File.Copy(LoaderDllPath, Path.Combine(BinariesDir, "WebView2Loader.dll"), true);
    }
  }
}