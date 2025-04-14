cmake_minimum_required(VERSION 3.15)
project(simple_lib)

# 设置构建类型为动态库
add_library(simple_lib SHARED
    simple_lib.c
)

# 设置输出目录
set_target_properties(simple_lib PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)