#include "builtins.h"

#include <stdexcept>

static std::vector<double> CollectNumbers(const Value& args) {
  std::vector<double> nums;
  auto cur = args;
  while (std::holds_alternative<std::shared_ptr<Pair>>(cur)) {
    auto cell = std::get<std::shared_ptr<Pair>>(cur);
    nums.push_back(std::get<double>(cell->car));
    cur = cell->cdr;
  }
  return nums;
}

Value BuiltinArith(const std::string& op, const Value& args) {
  auto nums = CollectNumbers(args);
  if (nums.empty()) {
    throw std::runtime_error(op + " requires at least 1 argument");
  }
  if (op == "+") {
    double sum = 0;
    for (auto n : nums)
      sum += n;
    return sum;
  }
  if (op == "*") {
    double prod = 1;
    for (auto n : nums)
      prod *= n;
    return prod;
  }
  if (op == "-") {
    if (nums.size() == 1)
      return -nums[0];
    return nums[0] - nums[1];
  }
  if (op == "/") {
    if (nums[1] == 0)
      throw std::runtime_error("division by zero");
    return nums[0] / nums[1];
  }
  throw std::runtime_error("unknown arith op: " + op);
}

Value BuiltinListOp(const std::string& op, const Value& args) {
  auto cell = std::get<std::shared_ptr<Pair>>(args);
  if (op == "car")
    return std::get<std::shared_ptr<Pair>>(cell->car)->car;
  if (op == "cdr")
    return std::get<std::shared_ptr<Pair>>(cell->car)->cdr;
  if (op == "cons") {
    auto rest = std::get<std::shared_ptr<Pair>>(cell->cdr);
    auto pair = std::make_shared<Pair>();
    pair->car = cell->car;
    pair->cdr = rest->car;
    return pair;
  }
  if (op == "list") {
    Value result = Nil{};
    std::vector<Value> items;
    auto cur = args;
    while (std::holds_alternative<std::shared_ptr<Pair>>(cur)) {
      auto c = std::get<std::shared_ptr<Pair>>(cur);
      items.push_back(c->car);
      cur = c->cdr;
    }
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
      auto p = std::make_shared<Pair>();
      p->car = std::move(*it);
      p->cdr = result;
      result = p;
    }
    return result;
  }
  if (op == "null?") {
    return std::holds_alternative<Nil>(cell->car);
  }
  if (op == "pair?") {
    return std::holds_alternative<std::shared_ptr<Pair>>(cell->car);
  }
  throw std::runtime_error("unknown list op: " + op);
}

Value BuiltinCompare(const std::string& op, const Value& args) {
  auto cell = std::get<std::shared_ptr<Pair>>(args);
  auto rest = std::get<std::shared_ptr<Pair>>(cell->cdr);
  auto a = std::get<double>(cell->car);
  auto b = std::get<double>(rest->car);
  if (op == "=")
    return a == b;
  if (op == "<")
    return a < b;
  if (op == ">")
    return a > b;
  if (op == "<=")
    return a <= b;
  if (op == ">=")
    return a >= b;
  throw std::runtime_error("unknown compare op: " + op);
}

Value BuiltinBoolean(const std::string& op, const Value& args) {
  auto cell = std::get<std::shared_ptr<Pair>>(args);
  if (op == "not")
    return !IsTruthy(cell->car);
  throw std::runtime_error("unknown boolean op: " + op);
}
