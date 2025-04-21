cmake_minimum_required(VERSION 3.26)
project(MacGUIApp LANGUAGES CXX Swift)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 20)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

# 设置macOS部署目标版本
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")

# 借助conan安装C++依赖库
execute_process(
    COMMAND conan install . --output-folder=${CMAKE_BINARY_DIR} --build=missing 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# 设置 CMake 模块路径，让 CMake 能找到 Conan 生成的配置文件
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# 查找 spdlog 库
find_package(spdlog REQUIRED)

# 源文件
set(SOURCES
    src/main.cc
    src/SwiftBridge.mm
    src/AppDelegate.swift
    src/MainWindow.swift
)

# 创建可执行文件
add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${spdlog_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_compile_definitions(${PROJECT_NAME} PRIVATE SPDLOG_USE_STD_FORMAT)

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

# 取悦sourcekit-lsp
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${CMAKE_SOURCE_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to project root"
)