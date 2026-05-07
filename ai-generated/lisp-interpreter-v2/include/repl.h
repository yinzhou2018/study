#ifndef REPL_H_
#define REPL_H_

#include "evaluator.h"

class Repl {
 public:
  Repl();
  void Run();

 private:
  std::string ReadExpr();
  int CountParens(const std::string& line);
  void HandleExit();

  Evaluator evaluator_;
  bool running_ = true;
};

#endif  // REPL_H_
