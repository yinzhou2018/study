#pragma once

#include <vector>

#include "calculator/ast.h"
#include "calculator/result.h"
#include "calculator/token.h"

namespace calculator {

class Parser {
 public:
  explicit Parser(std::vector<Token> tokens);
  Result<Expr> Parse();

 private:
  std::vector<Token> tokens_;
  int pos_{0};

  const Token& Peek() const;
  Token Advance();
  bool Match(TokenType type);
  Result<Expr> ExprRule();
  Result<Expr> Term();
  Result<Expr> Factor();
};

}  // namespace calculator
