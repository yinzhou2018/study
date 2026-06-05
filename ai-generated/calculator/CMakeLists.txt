cmake_minimum_required(VERSION 3.20)
project(calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

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
