set(CMAKE_SYSTEM_NAME Android)

set(ANDROID_NDK "D:/android/sdk/ndk/21.4.7075529")

set(ANDROID_PLATFORM android-21)

set(ANDROID_ABI armeabi-v7a)

set(CMAKE_C_COMPILER ${ANDROID_NDK}/toolchains/llvm/prebuilt/windows-x86_64/bin/clang.exe)
set(CMAKE_CXX_COMPILER ${ANDROID_NDK}/toolchains/llvm/prebuilt/windows-x86_64/bin/clang++.exe)

set(CMAKE_FIND_ROOT_PATH ${ANDROID_NDK}/platforms/${ANDROID_PLATFORM}/arch-arm)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)