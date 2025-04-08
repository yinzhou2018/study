cmake_minimum_required(VERSION 3.26)
project(MacGUIApp LANGUAGES CXX Swift)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

# 设置macOS部署目标版本
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")

# 源文件
set(SOURCES
    src/main.cc
    src/SwiftBridge.mm
    src/AppDelegate.swift
    src/MainWindow.swift
)

# 创建可执行文件
add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES})

# 设置桥接头文件
set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.MacGUIApp"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.example.MacGUIApp"
    XCODE_ATTRIBUTE_SWIFT_EMIT_MODULE_INTERFACE YES
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES YES
    XCODE_ATTRIBUTE_SWIFT_INSTALL_OBJC_HEADER YES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist"
)

# 链接必要的框架
target_link_libraries(${PROJECT_NAME} PRIVATE
    "-framework Cocoa"
    "-framework Foundation"
)