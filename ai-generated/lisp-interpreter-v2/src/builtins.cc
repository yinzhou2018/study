#include "builtins.h"
#include "environment.h"

#include <stdexcept>

std::unordered_map<std::string, BuiltinManager::Handler> BuiltinManager::handlers_;

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

void BuiltinManager::InitRegistry() {
  static bool initialized = false;
  if (initialized)
    return;
  Register("+", BuiltinArith);
  Register("-", BuiltinArith);
  Register("*", BuiltinArith);
  Register("/", BuiltinArith);
  Register("car", BuiltinListOp);
  Register("cdr", BuiltinListOp);
  Register("cons", BuiltinListOp);
  Register("list", BuiltinListOp);
  Register("null?", BuiltinListOp);
  Register("pair?", BuiltinListOp);
  Register("=", BuiltinCompare);
  Register("<", BuiltinCompare);
  Register(">", BuiltinCompare);
  Register("<=", BuiltinCompare);
  Register(">=", BuiltinCompare);
  Register("not", BuiltinBoolean);
  initialized = true;
}

bool BuiltinManager::IsBuiltin(const std::string& name) {
  InitRegistry();
  return handlers_.find(name) != handlers_.end();
}

Value BuiltinManager::Call(const std::string& name, const Value& args) {
  InitRegistry();
  auto it = handlers_.find(name);
  if (it == handlers_.end())
    throw std::runtime_error("not a builtin: " + name);
  return it->second(name, args);
}

void BuiltinManager::RegisterAll(std::shared_ptr<Environment> env) {
  InitRegistry();
  for (const auto& [name, _] : handlers_) {
    env->Define(name, Symbol{name});
  }
}

void BuiltinManager::Register(const std::string& name, Handler handler) {
  handlers_[name] = std::move(handler);
}
