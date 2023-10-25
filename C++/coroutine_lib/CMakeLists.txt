cmake_minimum_required(VERSION 3.20.0)
project(coroutine_study VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)

set(SRC_LIST main.cpp stack_coroutine.cpp)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
  list(APPEND SRC_LIST stack_allocator_win.cpp)
else()
  list(APPEND SRC_LIST stack_allocator_posix.cpp)
endif()

add_executable(coroutine_study ${SRC_LIST})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
