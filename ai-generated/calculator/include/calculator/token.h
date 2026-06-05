#pragma once

#include <string>
#include <variant>

namespace calculator {

enum class TokenType {
  Number,
  Plus,
  Minus,
  Star,
  Slash,
  LeftParen,
  RightParen,
  EndOfInput,
};

struct Token {
  TokenType type;
  double value;
  int position;
};

}  // namespace calculator
