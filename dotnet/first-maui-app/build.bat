echo off
echo Android Sdk Location: %ANDROID_HOME%
dotnet build -p:AndroidSdkDirectory="%ANDROID_HOME%"