add_executable(calculator main.cc lexer.cc parser.cc evaluator.cc repl.cc)

target_include_directories(calculator PRIVATE ${CMAKE_SOURCE_DIR}/include
                                              ${CMAKE_SOURCE_DIR}/src)
