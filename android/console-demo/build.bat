@echo off

set DEF_ANDROID_NDK=D:/android/sdk/ndk/21.4.7075529
if defined ANDROID_NDK (
  echo -- ANDROID_NDK enviroment variable found: %ANDROID_NDK%
) else (
  echo -- ANDROID_NDK enviroment variable not found, use default: %DEF_ANDROID_NDK%
  set ANDROID_NDK=%DEF_ANDROID_NDK%
)

set CMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%/build/cmake/android.toolchain.cmake
set ANDROID_ABI=arm64-v8a
set ANDROID_PLATFORM=android-21

set BUILD_PATH=build
if exist %BUILD_PATH% rmdir /s /q %BUILD_PATH%
mkdir %BUILD_PATH%
cd %BUILD_PATH%
cmake .. -DCMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN_FILE% -DANDROID_ABI=%ANDROID_ABI% -DANDROID_PLATFORM=%ANDROID_PLATFORM% -G "Ninja"
cmake --build .

@REM 拷贝到设备指定路径,修改为可执行权限并运行，注意android设备实验下来只有下面路径可以把文件修改为执行权限
set DEST_PATH=/data/local/tmp
set CONSOLE_APP=console-demo
echo -- Copy %CONSOLE_APP% to device path: %DEST_PATH% ...
adb push ./%CONSOLE_APP% %DEST_PATH%
adb shell chmod 777 %DEST_PATH%/%CONSOLE_APP%
echo -- Run %CONSOLE_APP% on device ...
adb shell %DEST_PATH%/%CONSOLE_APP%

cd ..