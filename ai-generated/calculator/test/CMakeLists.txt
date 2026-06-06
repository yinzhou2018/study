include(FetchContent)
FetchContent_Declare(googletest GIT_REPOSITORY
                     https://github.com/google/googletest.git GIT_TAG v1.14.0)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

target_compile_options(gtest PRIVATE -Wno-character-conversion)
target_compile_options(gtest_main PRIVATE -Wno-character-conversion)

add_executable(calculator_test result_test.cc token_test.cc ast_test.cc
                                lexer_test.cc parser_test.cc
                                evaluator_test.cc repl_test.cc
                                input_reader_test.cc linenoise_reader_test.cc
                                integration_test.cc
                                ${CMAKE_SOURCE_DIR}/src/lexer.cc
                                ${CMAKE_SOURCE_DIR}/src/parser.cc
                                ${CMAKE_SOURCE_DIR}/src/evaluator.cc
                                ${CMAKE_SOURCE_DIR}/src/repl.cc
                                ${CMAKE_SOURCE_DIR}/src/input_reader.cc)

target_link_libraries(calculator_test PRIVATE GTest::gtest_main linenoise)

target_include_directories(calculator_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
gtest_discover_tests(calculator_test)
