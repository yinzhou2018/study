cmake_minimum_required(VERSION 3.0.0)
project(vis_test VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(sta_lib STATIC lib.cpp)
add_library(dyn_lib SHARED dyn_lib.cpp)
add_executable(vis_test main.cpp add.cpp)

# target_link_directories(dyn_lib PUBLIC build)
target_link_libraries(vis_test dyn_lib sta_lib)
target_link_libraries(dyn_lib sta_lib)

# set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}   -fvisibility=hidden")
# set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -fvisibility=hidden")
# set(CMAKE_C_FLAGS_RELEASE     "${CMAKE_C_FLAGS_RELEASE}   -fvisibility=hidden")
# set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
