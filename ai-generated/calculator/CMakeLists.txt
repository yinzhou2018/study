cmake_minimum_required(VERSION 3.20)
project(calculator LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

include(FetchContent)
FetchContent_Declare(
  linenoise
  GIT_REPOSITORY https://github.com/antirez/linenoise.git
  GIT_TAG master)
FetchContent_MakeAvailable(linenoise)

if(NOT TARGET linenoise)
  add_library(linenoise STATIC ${linenoise_SOURCE_DIR}/linenoise.c)
  target_include_directories(linenoise PUBLIC ${linenoise_SOURCE_DIR})
  target_compile_options(linenoise PRIVATE -w)
endif()

add_subdirectory(src)

enable_testing()
add_subdirectory(test)

find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
  file(GLOB_RECURSE FORMAT_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cc
       ${CMAKE_SOURCE_DIR}/src/*.h
       ${CMAKE_SOURCE_DIR}/test/*.cc)
  add_custom_target(format COMMAND ${CLANG_FORMAT} -i ${FORMAT_SOURCES}
                    COMMENT "Formatting code with clang-format")
endif()
