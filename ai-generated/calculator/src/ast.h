#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace calculator {

struct NumberExpr {
  double value;
};

struct ConstantExpr {
  std::string name;
};

struct BinaryExpr;
struct UnaryExpr;
struct FuncCallExpr;

using Expr = std::variant<NumberExpr,
                          std::unique_ptr<BinaryExpr>,
                          std::unique_ptr<UnaryExpr>,
                          std::unique_ptr<FuncCallExpr>,
                          ConstantExpr>;

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

struct FuncCallExpr {
  std::string name;
  std::vector<Expr> args;
};

}  // namespace calculator
