cmake_minimum_required(VERSION 3.20.0)
project(playground VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Statically link the C/C++ runtime libraries
if (MSVC)
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
elseif (MINGW)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -static-libstdc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()

file(GLOB_RECURSE DIR_SRCS RELATIVE ${CMAKE_SOURCE_DIR} *.cpp *.c *.cc)
list(FILTER DIR_SRCS EXCLUDE REGEX "^build/")
if (NOT MINGW)
  add_executable(playground ${DIR_SRCS})
else()
  add_executable(playground main.cpp)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(playground pthread)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
