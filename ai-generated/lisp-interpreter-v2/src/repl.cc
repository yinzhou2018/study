#include "repl.h"

#include "parser.h"

#include <iostream>
#include <string>

#ifdef HAS_READLINE
#include <readline/history.h>
#include <readline/readline.h>
#endif

Repl::Repl() = default;

void Repl::Run() {
  std::cout << "Lisp Interpreter v1.0\n";
  std::cout << "Type (exit) to quit.\n";

  while (running_) {
    auto input = ReadExpr();
    if (input.empty())
      continue;

    try {
      auto exprs = Parse(input);
      for (auto& expr : exprs) {
        auto result = evaluator_.Eval(expr);
        std::cout << ValueToString(result) << "\n";
      }
    } catch (const std::exception& e) {
      std::cerr << "error: " << e.what() << "\n";
    }
  }
}

std::string Repl::ReadExpr() {
  std::string expr;
  int depth = 0;

  while (true) {
    std::string prompt = depth == 0 ? "> " : "... ";
    std::string line;

#ifdef HAS_READLINE
    auto* raw = readline(prompt.c_str());
    if (!raw) {
      std::cout << "\n";
      running_ = false;
      return "";
    }
    line = raw;
    free(raw);
    if (!line.empty())
      add_history(line.c_str());
#else
    std::cout << prompt;
    if (!std::getline(std::cin, line)) {
      std::cout << "\n";
      running_ = false;
      return "";
    }
#endif

    depth += CountParens(line);
    if (!expr.empty())
      expr += " ";
    expr += line;

    if (depth <= 0)
      break;
  }

  if (expr == "exit" || expr == "quit") {
    HandleExit();
    return "";
  }
  return expr;
}

int Repl::CountParens(const std::string& line) {
  int count = 0;
  bool in_string = false;
  bool in_comment = false;
  for (size_t i = 0; i < line.size(); ++i) {
    if (in_comment)
      continue;
    if (in_string) {
      if (line[i] == '\\' && i + 1 < line.size()) {
        ++i;
        continue;
      }
      if (line[i] == '"')
        in_string = false;
      continue;
    }
    if (line[i] == ';') {
      in_comment = true;
      continue;
    }
    if (line[i] == '"') {
      in_string = true;
      continue;
    }
    if (line[i] == '(')
      ++count;
    if (line[i] == ')')
      --count;
  }
  return count;
}

void Repl::HandleExit() {
  std::cout << "Bye!\n";
  running_ = false;
}
