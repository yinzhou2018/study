#pragma once

#include <memory>
#include <variant>

namespace calculator {

struct NumberExpr {
  double value;
};

struct BinaryExpr;
struct UnaryExpr;

using Expr = std::variant<NumberExpr,
                          std::unique_ptr<BinaryExpr>,
                          std::unique_ptr<UnaryExpr>>;

struct BinaryExpr {
  enum Op { Add, Sub, Mul, Div };
  Op op;
  Expr left;
  Expr right;
};

struct UnaryExpr {
  enum Op { Negate };
  Op op;
  Expr operand;
};

}  // namespace calculator
