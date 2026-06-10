#include "parser.h"

#include <memory>
#include <utility>

namespace calculator {

Parser::Parser(std::vector<Token> tokens) : tokens_{std::move(tokens)} {}

const Token& Parser::Peek() const {
  return tokens_[pos_];
}

Token Parser::Advance() {
  return tokens_[pos_++];
}

bool Parser::Match(TokenType type) {
  if (Peek().type == type) {
    Advance();
    return true;
  }
  return false;
}

Result<Expr> Parser::Parse() {
  return ExprRule();
}

Result<Expr> Parser::ExprRule() {
  auto left = Term();
  if (!left.ok())
    return left.error();

  while (Peek().type == TokenType::Plus || Peek().type == TokenType::Minus) {
    auto op =
        Peek().type == TokenType::Plus ? BinaryExpr::Add : BinaryExpr::Sub;
    Advance();
    auto right = Term();
    if (!right.ok())
      return right.error();
    auto node =
        std::make_unique<BinaryExpr>(BinaryExpr{op, left.Take(), right.Take()});
    left = Result<Expr>(Expr(std::move(node)));
  }
  return left;
}

Result<Expr> Parser::Term() {
  auto left = Factor();
  if (!left.ok())
    return left.error();

  while (Peek().type == TokenType::Star || Peek().type == TokenType::Slash) {
    auto op =
        Peek().type == TokenType::Star ? BinaryExpr::Mul : BinaryExpr::Div;
    Advance();
    auto right = Factor();
    if (!right.ok())
      return right.error();
    auto node =
        std::make_unique<BinaryExpr>(BinaryExpr{op, left.Take(), right.Take()});
    left = Result<Expr>(Expr(std::move(node)));
  }
  return left;
}

Result<Expr> Parser::Factor() {
  if (Peek().type == TokenType::Minus) {
    Advance();
    auto operand = Factor();
    if (!operand.ok())
      return operand.error();
    auto node = std::make_unique<UnaryExpr>(
        UnaryExpr{UnaryExpr::Negate, operand.Take()});
    return Result<Expr>(Expr(std::move(node)));
  }
  if (Peek().type == TokenType::Number) {
    auto value = Peek().value;
    Advance();
    return Expr(NumberExpr{value});
  }
  if (Match(TokenType::LeftParen)) {
    auto expr = ExprRule();
    if (!expr.ok())
      return expr.error();
    if (!Match(TokenType::RightParen)) {
      return Error{"Mismatched parentheses"};
    }
    return expr;
  }
  if (Peek().type == TokenType::Identifier) {
    std::string name = Peek().text;
    int name_pos = Peek().position;
    Advance();
    if (Peek().type == TokenType::LeftParen) {
      Advance();
      auto func = std::make_unique<FuncCallExpr>(FuncCallExpr{name, {}});
      if (Peek().type == TokenType::RightParen) {
        Advance();
        return Result<Expr>(Expr(std::move(func)));
      }
      auto first_arg = ExprRule();
      if (!first_arg.ok())
        return first_arg.error();
      func->args.push_back(first_arg.Take());
      while (Peek().type == TokenType::Comma) {
        Advance();
        auto arg = ExprRule();
        if (!arg.ok())
          return arg.error();
        func->args.push_back(arg.Take());
      }
      if (!Match(TokenType::RightParen)) {
        return Error{"Mismatched parentheses in function call '" + name +
                     "' at position " + std::to_string(name_pos)};
      }
      return Result<Expr>(Expr(std::move(func)));
    }
    return Expr(ConstantExpr{std::move(name)});
  }
  return Error{"Unexpected token at position " +
               std::to_string(Peek().position)};
}

}  // namespace calculator
