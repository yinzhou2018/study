cmake_minimum_required(VERSION 3.0.0)
project(exception_study VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(exception_study main.cpp)

# 设置C编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align")

# 设置C++编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
