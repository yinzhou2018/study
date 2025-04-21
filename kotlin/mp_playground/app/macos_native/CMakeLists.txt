cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 20)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

project(macos_lib)

add_library(macos_lib SHARED macos_lib.mm)

if(OUTPUT_DIR)
  message(STATUS "OUTPUT_DIR: ${OUTPUT_DIR}")
  set(output_dir ${OUTPUT_DIR})
else()
  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" system_processor_lower)
  if(system_processor_lower MATCHES "^(x86_64|amd64)$")
    set(targetName "macosX64")
  else()
    set(targetName "macosArm64")
  endif()
  set(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/../build/bin/${targetName}/debugExecutable)
  message(STATUS "OUTPUT_DIR is not set. Use default settings: ${output_dir}")
endif()

target_link_libraries(macos_lib PRIVATE "-framework Foundation")

# 设置输出目录
set_target_properties(macos_lib PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
    LIBRARY_OUTPUT_DIRECTORY "${output_dir}"
    ARCHIVE_OUTPUT_DIRECTORY "${output_dir}"
)

# 设置 framework 相关属性
set_target_properties(macos_lib PROPERTIES
    FRAMEWORK TRUE  # 标记为 framework
    FRAMEWORK_VERSION A  # framework 版本
    MACOSX_FRAMEWORK_IDENTIFIER com.example.macos_lib  # framework 标识符
    MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist  # Info.plist 文件路径
    PUBLIC_HEADER mac_lib.h  # 公开头文件
    VERSION 1.0.0  # 版本号
    SOVERSION 1  # 动态库版本号
)