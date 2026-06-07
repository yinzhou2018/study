#include "evaluator.h"

#include <cmath>
#include <functional>
#include <unordered_map>
#include <variant>

namespace calculator {
namespace evaluator {

namespace {

using FuncImpl = std::function<double(const std::vector<double>&)>;

struct FuncDef {
  int expected_args;
  FuncImpl impl;
};

const std::unordered_map<std::string, FuncDef>& GetFunctionTable() {
  static const std::unordered_map<std::string, FuncDef> table = {
      {"sin", {1, [](const auto& a) { return std::sin(a[0]); }}},
      {"cos", {1, [](const auto& a) { return std::cos(a[0]); }}},
      {"tan", {1, [](const auto& a) { return std::tan(a[0]); }}},
      {"asin", {1, [](const auto& a) { return std::asin(a[0]); }}},
      {"acos", {1, [](const auto& a) { return std::acos(a[0]); }}},
      {"atan", {1, [](const auto& a) { return std::atan(a[0]); }}},
      {"sqrt", {1, [](const auto& a) { return std::sqrt(a[0]); }}},
      {"log", {1, [](const auto& a) { return std::log10(a[0]); }}},
      {"ln", {1, [](const auto& a) { return std::log(a[0]); }}},
      {"abs", {1, [](const auto& a) { return std::abs(a[0]); }}},
      {"ceil", {1, [](const auto& a) { return std::ceil(a[0]); }}},
      {"floor", {1, [](const auto& a) { return std::floor(a[0]); }}},
      {"pow", {2, [](const auto& a) { return std::pow(a[0], a[1]); }}},
  };
  return table;
}

Result<double> ValidateDomain(const std::string& name,
                              const std::vector<double>& args) {
  if (name == "sqrt" && args[0] < 0) {
    return Error{"sqrt argument must be non-negative"};
  }
  if ((name == "log" || name == "ln") && args[0] <= 0) {
    return Error{name + std::string(" argument must be positive")};
  }
  if (name == "asin" && (args[0] < -1 || args[0] > 1)) {
    return Error{"asin argument must be between -1 and 1"};
  }
  if (name == "acos" && (args[0] < -1 || args[0] > 1)) {
    return Error{"acos argument must be between -1 and 1"};
  }
  return args[0];
}

Result<double> VisitFuncCall(const FuncCallExpr& expr) {
  auto& table = GetFunctionTable();
  auto it = table.find(expr.name);
  if (it == table.end()) {
    return Error{"Unknown function: " + expr.name};
  }
  if (static_cast<int>(expr.args.size()) != it->second.expected_args) {
    return Error{expr.name + " expects " +
                 std::to_string(it->second.expected_args) + " argument(s)"};
  }

  std::vector<double> args;
  for (const auto& arg : expr.args) {
    auto val = Evaluate(arg);
    if (!val.ok())
      return val.error();
    args.push_back(val.value());
  }

  auto domain = ValidateDomain(expr.name, args);
  if (!domain.ok())
    return domain.error();

  return it->second.impl(args);
}

Result<double> VisitConstant(const ConstantExpr& expr) {
  static const std::unordered_map<std::string, double> table = {
      {"pi", M_PI},
      {"e", M_E},
  };
  auto it = table.find(expr.name);
  if (it == table.end()) {
    return Error{"Unknown constant: " + expr.name};
  }
  return it->second;
}

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
        } else if constexpr (std::is_same_v<T,
                                            std::unique_ptr<FuncCallExpr>>) {
          return VisitFuncCall(*node);
        } else if constexpr (std::is_same_v<T, ConstantExpr>) {
          return VisitConstant(node);
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
