#pragma once

#include <string>
#include <vector>

#include "result.h"
#include "token.h"

namespace calculator {

class Lexer {
 public:
  explicit Lexer(std::string input);
  Result<std::vector<Token>> Tokenize();

 private:
  std::string input_;
  int pos_{0};

  char Peek() const;
  char Advance();
  void SkipWhitespace();
  Result<Token> ReadNumber();
};

}  // namespace calculator
