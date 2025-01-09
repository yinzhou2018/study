cmake_minimum_required(VERSION 3.20.0)
project(playground VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)

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

aux_source_directory(. DIR_SRCS)
if (NOT MINGW)
  add_executable(playground ${DIR_SRCS})
else()
  add_executable(playground main.cpp)
endif()


if (NOT WIN32)
add_library(utils SHARED lib/utils.cc)
target_link_libraries(playground utils)
endif()

# 判断MAC平台
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  target_link_libraries(playground ${COREFOUNDATION_LIBRARY})
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
