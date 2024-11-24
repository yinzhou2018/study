cmake_minimum_required(VERSION 3.20.0)
project(playground VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)

add_library(utils SHARED lib/utils.cc)
aux_source_directory(. DIR_SRCS)
add_executable(playground ${DIR_SRCS})
target_link_libraries(playground utils)

set_target_properties(playground PROPERTIES
    INSTALL_RPATH "@executable_path; @loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# 判断MAC平台
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  target_link_libraries(playground ${COREFOUNDATION_LIBRARY})
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
