cmake_minimum_required(VERSION 3.20)
project(lisp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find flex and bison
find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

# Find readline (optional)
include(CheckIncludeFileCXX)
check_include_file_cxx("readline/readline.h" HAS_READLINE_H)

# Generate lexer from flex
FLEX_TARGET(LispLexer src/lisp.l ${CMAKE_CURRENT_BINARY_DIR}/lisp.lex.cc)

# Generate parser from bison
BISON_TARGET(LispParser src/lisp.y ${CMAKE_CURRENT_BINARY_DIR}/lisp.tab.cc
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lisp.tab.h)

add_executable(lisp
    src/main.cc
    src/value.cc
    src/environment.cc
    src/evaluator.cc
    src/builtins.cc
    src/repl.cc
    src/parser.cc
    ${FLEX_LispLexer_OUTPUTS}
    ${BISON_LispParser_OUTPUTS}
)

target_include_directories(lisp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(HAS_READLINE_H)
    target_compile_definitions(lisp PRIVATE HAS_READLINE)
    target_link_libraries(lisp PRIVATE readline)
endif()
