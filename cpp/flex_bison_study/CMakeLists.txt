cmake_minimum_required(VERSION 3.20.0)
project(flex_bison_study VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)

execute_process(COMMAND flex -o build/lex.yy.c fb.l COMMAND bison -d -o build/fb.tab.c fb.y WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

aux_source_directory(build DIR_SRCS)
aux_source_directory(./ DIR_SRCS2)
add_executable(flex_bison_study ${DIR_SRCS2} ${DIR_SRCS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
