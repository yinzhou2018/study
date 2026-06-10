#include "evaluator.h"
#include "repl.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  if (argc > 1) {
    std::string expr;
    for (int i = 1; i < argc; ++i) {
      if (i > 1)
        expr += ' ';
      expr += argv[i];
    }

    auto result = calculator::evaluator::EvaluateExpression(expr);
    if (!result.ok()) {
      std::cerr << result.error().message << std::endl;
      return 1;
    }
    std::cout << result.value() << std::endl;
    return 0;
  }

  calculator::Repl repl;
  repl.Run();
  return 0;
}
