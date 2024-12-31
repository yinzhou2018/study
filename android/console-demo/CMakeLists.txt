cmake_minimum_required(VERSION 3.5.0)

project(console-demo VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(utils SHARED utils.cpp)
add_executable(console-demo main.cpp)
target_link_libraries(console-demo utils)

   
