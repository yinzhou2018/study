#!/bin/bash

# 设置默认的ANDROID_NDK路径
DEF_ANDROID_NDK="/Users/yinzhou/Library/Android/sdk/ndk/25.2.9519653"
# 检查是否定义了ANDROID_NDK环境变量
if [ -z "$ANDROID_NDK" ]; then
    echo "-- ANDROID_NDK environment variable not found, using default: $DEF_ANDROID_NDK"
    export ANDROID_NDK=$DEF_ANDROID_NDK
else
    echo "-- ANDROID_NDK environment variable found: $ANDROID_NDK"
fi

# 设置CMAKE_TOOLCHAIN_FILE路径
export CMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake"

# 设置目标ABI和平台版本
export ANDROID_ABI="arm64-v8a"
export ANDROID_PLATFORM="android-21"

# 定义构建目录
BUILD_PATH="build"

# 如果构建目录已存在，则删除并重新创建
if [ -d "$BUILD_PATH" ]; then
    rm -rf "$BUILD_PATH"
fi
mkdir "$BUILD_PATH"
cd "$BUILD_PATH"

# 运行cmake配置和构建
cmake .. -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" -DANDROID_ABI="$ANDROID_ABI" -DANDROID_PLATFORM="$ANDROID_PLATFORM" -G "Ninja"
cmake --build .

# 定义目标设备路径和应用名称
DEST_PATH="/data/local/tmp"
CONSOLE_APP="console-demo"
UTILS_LIB="libutils.so"

# 将应用拷贝到设备并设置执行权限，然后运行
echo "-- Copy $CONSOLE_APP and $UTILS_LIB to device path: $DEST_PATH ..."
adb push "./$CONSOLE_APP" "$DEST_PATH"
adb push "./$UTILS_LIB" "$DEST_PATH"
adb shell chmod 777 "$DEST_PATH/$CONSOLE_APP"
echo "-- Run $CONSOLE_APP on device ..."

# android平台不支持原生linux构建的rpath设置，手动设置LD_LIBRARY_PATH环境变量来支持动态库的加载
adb shell export LD_LIBRARY_PATH=/data/local/tmp:$LD_LIBRARY_PATH; "$DEST_PATH/$CONSOLE_APP"

cd ..