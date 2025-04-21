cmake_minimum_required(VERSION 3.15)
project(simple_lib)

if(OUTPUT_DIR)
  message(STATUS "OUTPUT_DIR: ${OUTPUT_DIR}")
  set(output_dir ${OUTPUT_DIR})
else()
  message(STATUS "OUTPUT_DIR is not set. Use default settings.")
  if (WIN32)
    set(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/../build/bin/mingwX64/debugExecutable)
  else ()
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" system_processor_lower)
    if(system_processor_lower MATCHES "^(x86_64|amd64)$")
      set(targetName "macosX64")
    else()
      set(targetName "macosArm64")
    endif()
    set(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/../build/bin/${targetName}/debugExecutable)
  endif()
endif()

# 设置构建类型为动态库
add_library(simple_lib SHARED
    simple_lib.c
)

# 设置输出目录
set_target_properties(simple_lib PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${output_dir}"
    LIBRARY_OUTPUT_DIRECTORY "${output_dir}"
    ARCHIVE_OUTPUT_DIRECTORY "${output_dir}"
)