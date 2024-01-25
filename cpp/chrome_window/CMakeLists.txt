cmake_minimum_required(VERSION 3.20.0)
project(chrome_window VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(chrome_window WIN32 main.cpp app.rc)

target_link_libraries(chrome_window dwmapi d2d1 d3d11 dcomp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
