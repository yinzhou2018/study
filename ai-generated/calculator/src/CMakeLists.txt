add_executable(calculator main.cc lexer.cc parser.cc evaluator.cc repl.cc
                              input_reader.cc)

target_include_directories(calculator PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(calculator PRIVATE linenoise)
