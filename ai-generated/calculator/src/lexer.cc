#include "lexer.h"

#include <cctype>

namespace calculator {

Lexer::Lexer(std::string input) : input_{std::move(input)} {}

char Lexer::Peek() const {
  return pos_ < static_cast<int>(input_.size()) ? input_[pos_] : '\0';
}

char Lexer::Advance() {
  return pos_ < static_cast<int>(input_.size()) ? input_[pos_++] : '\0';
}

void Lexer::SkipWhitespace() {
  while (std::isspace(Peek())) {
    Advance();
  }
}

Result<Token> Lexer::ReadNumber() {
  int start = pos_;
  while (std::isdigit(Peek())) {
    Advance();
  }
  if (Peek() == '.') {
    Advance();
    while (std::isdigit(Peek())) {
      Advance();
    }
  }
  double value = std::stod(input_.substr(start, pos_ - start));
  return Token{TokenType::Number, value, start};
}

Result<std::vector<Token>> Lexer::Tokenize() {
  std::vector<Token> tokens;
  while (pos_ < static_cast<int>(input_.size())) {
    SkipWhitespace();
    if (pos_ >= static_cast<int>(input_.size()))
      break;

    char ch = Peek();
    if (std::isdigit(ch)) {
      auto result = ReadNumber();
      if (!result.ok())
        return result.error();
      tokens.push_back(result.value());
    } else if (ch == '+') {
      tokens.push_back(Token{TokenType::Plus, 0, pos_});
      Advance();
    } else if (ch == '-') {
      tokens.push_back(Token{TokenType::Minus, 0, pos_});
      Advance();
    } else if (ch == '*') {
      tokens.push_back(Token{TokenType::Star, 0, pos_});
      Advance();
    } else if (ch == '/') {
      tokens.push_back(Token{TokenType::Slash, 0, pos_});
      Advance();
    } else if (ch == '(') {
      tokens.push_back(Token{TokenType::LeftParen, 0, pos_});
      Advance();
    } else if (ch == ')') {
      tokens.push_back(Token{TokenType::RightParen, 0, pos_});
      Advance();
    } else {
      return Error{"Illegal character '" + std::string(1, ch) +
                   "' at position " + std::to_string(pos_)};
    }
  }
  tokens.push_back(Token{TokenType::EndOfInput, 0, pos_});
  return tokens;
}

}  // namespace calculator
