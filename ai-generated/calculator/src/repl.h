#pragma once

#include <functional>
#include <iostream>
#include <string>

#include "calculator/result.h"
#include "evaluator.h"
#include "lexer.h"
#include "parser.h"

namespace calculator {

class Repl {
 public:
  explicit Repl(std::istream& in = std::cin, std::ostream& out = std::cout);

  void Run();

 private:
  std::reference_wrapper<std::istream> in_;
  std::reference_wrapper<std::ostream> out_;
  bool interactive_;

  void ProcessLine(const std::string& line);
  bool IsExitCommand(const std::string& line) const;
};

}  // namespace calculator
