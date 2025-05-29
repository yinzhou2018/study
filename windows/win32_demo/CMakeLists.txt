cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(Win32Demo)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(Win32Demo WIN32 src/main.cpp)

add_compile_definitions(_UNICODE UNICODE)
