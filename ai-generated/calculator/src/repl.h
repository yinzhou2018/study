#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "evaluator.h"
#include "input_reader.h"
#include "lexer.h"
#include "parser.h"
#include "result.h"

namespace calculator {

class Repl {
 public:
  explicit Repl(std::istream& in = std::cin, std::ostream& out = std::cout);

  void Run();

 private:
  std::unique_ptr<InputReader> reader_;
  std::reference_wrapper<std::ostream> out_;

  void ProcessLine(const std::string& line);
  bool IsExitCommand(const std::string& line) const;
};

}  // namespace calculator
