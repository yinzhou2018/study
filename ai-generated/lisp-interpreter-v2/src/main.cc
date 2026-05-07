#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "evaluator.h"
#include "parser.h"
#include "repl.h"
#include "value.h"

int main(int argc, char* argv[]) {
  if (argc > 2) {
    std::cerr << "Usage: lisp [file]\n";
    return 1;
  }

  if (argc == 1) {
    Repl repl;
    repl.Run();
    return 0;
  }

  const char* filename = argv[1];
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "error: file not found: " << filename << "\n";
    return 1;
  }

  std::ostringstream oss;
  oss << file.rdbuf();
  std::string source = oss.str();

  try {
    auto exprs = Parse(source);
    Evaluator evaluator;
    for (auto& expr : exprs) {
      auto result = evaluator.Eval(expr);
      if (!std::holds_alternative<Nil>(result) || std::holds_alternative<std::shared_ptr<Pair>>(result)) {
        std::cout << ValueToString(result) << "\n";
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
