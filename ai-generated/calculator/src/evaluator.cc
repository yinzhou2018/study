#include "evaluator.h"

#include <variant>

namespace calculator {
namespace evaluator {

namespace {

Result<double> VisitBinary(const BinaryExpr& expr);
Result<double> VisitUnary(const UnaryExpr& expr);

}  // namespace

Result<double> Evaluate(const Expr& expr) {
  return std::visit(
      [](const auto& node) -> Result<double> {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, NumberExpr>) {
          return node.value;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<BinaryExpr>>) {
          return VisitBinary(*node);
        } else if constexpr (std::is_same_v<T, std::unique_ptr<UnaryExpr>>) {
          return VisitUnary(*node);
        }
      },
      expr);
}

namespace {

Result<double> VisitBinary(const BinaryExpr& expr) {
  auto left = Evaluate(expr.left);
  if (!left.ok())
    return left.error();
  auto right = Evaluate(expr.right);
  if (!right.ok())
    return right.error();

  switch (expr.op) {
    case BinaryExpr::Add:
      return left.value() + right.value();
    case BinaryExpr::Sub:
      return left.value() - right.value();
    case BinaryExpr::Mul:
      return left.value() * right.value();
    case BinaryExpr::Div:
      if (right.value() == 0)
        return Error{"Division by zero"};
      return left.value() / right.value();
  }
}

Result<double> VisitUnary(const UnaryExpr& expr) {
  auto operand = Evaluate(expr.operand);
  if (!operand.ok())
    return operand.error();

  switch (expr.op) {
    case UnaryExpr::Negate:
      return -operand.value();
  }
}

}  // namespace

}  // namespace evaluator
}  // namespace calculator
