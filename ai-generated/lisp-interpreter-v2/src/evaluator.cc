#include "evaluator.h"

#include "builtins.h"

#include <stdexcept>

namespace {

bool IsSpecialForm(const std::string& name) {
  return name == "define" || name == "if" || name == "cond" || name == "let" || name == "quote" || name == "lambda" ||
         name == "and" || name == "or";
}

}  // namespace

Evaluator::Evaluator() : global_env_(std::make_shared<Environment>()) {
  BuiltinManager::RegisterAll(global_env_);
}

Value Evaluator::Eval(const Value& expr) {
  return EvalExpr(expr, global_env_);
}

Value Evaluator::EvalExpr(const Value& expr, std::shared_ptr<Environment> env) {
  if (std::holds_alternative<Nil>(expr) || std::holds_alternative<double>(expr) || std::holds_alternative<bool>(expr) ||
      std::holds_alternative<std::string>(expr)) {
    return expr;
  }
  if (std::holds_alternative<Symbol>(expr)) {
    return env->Lookup(std::get<Symbol>(expr).name);
  }
  if (!std::holds_alternative<std::shared_ptr<Pair>>(expr))
    return expr;

  auto pair = std::get<std::shared_ptr<Pair>>(expr);
  auto head = pair->car;
  if (std::holds_alternative<Symbol>(head)) {
    auto name = std::get<Symbol>(head).name;
    if (IsSpecialForm(name)) {
      return ApplySpecialForm(Symbol{name}, pair->cdr, env);
    }
  }
  auto func = EvalExpr(head, env);
  auto args = EvalList(pair->cdr, env);
  return ApplyFunction(func, args);
}

Value Evaluator::EvalList(const Value& exprs, std::shared_ptr<Environment> env) {
  if (std::holds_alternative<Nil>(exprs))
    return Nil{};
  auto pair = std::get<std::shared_ptr<Pair>>(exprs);
  auto val = EvalExpr(pair->car, env);
  auto rest = EvalList(pair->cdr, env);
  auto cell = std::make_shared<Pair>();
  cell->car = val;
  cell->cdr = rest;
  return cell;
}

Value Evaluator::ApplyFunction(const Value& func, const Value& args) {
  if (std::holds_alternative<std::shared_ptr<Lambda>>(func)) {
    auto lambda = std::get<std::shared_ptr<Lambda>>(func);
    auto call_env = std::make_shared<Environment>(lambda->closure);
    auto cur = args;
    for (auto& p : lambda->params) {
      if (std::holds_alternative<Nil>(cur)) {
        throw std::runtime_error("too few arguments");
      }
      auto cell = std::get<std::shared_ptr<Pair>>(cur);
      call_env->Define(p, cell->car);
      cur = cell->cdr;
    }
    return EvalExpr(lambda->body, call_env);
  }
  if (std::holds_alternative<Symbol>(func)) {
    auto name = std::get<Symbol>(func).name;
    if (BuiltinManager::IsBuiltin(name))
      return BuiltinManager::Call(name, args);
  }
  throw std::runtime_error("not a function: " + ValueToString(func));
}

Value Evaluator::ApplySpecialForm(const Symbol& name, const Value& args, std::shared_ptr<Environment> env) {
  auto n = name.name;
  if (n == "define")
    return EvalDefine(args, env);
  if (n == "if")
    return EvalIf(args, env);
  if (n == "cond")
    return EvalCond(args, env);
  if (n == "let")
    return EvalLet(args, env);
  if (n == "quote") {
    return std::get<std::shared_ptr<Pair>>(args)->car;
  }
  if (n == "lambda")
    return EvalLambda(args, env);
  if (n == "and") {
    Value result = true;
    auto cur = args;
    while (std::holds_alternative<std::shared_ptr<Pair>>(cur)) {
      auto cell = std::get<std::shared_ptr<Pair>>(cur);
      result = EvalExpr(cell->car, env);
      if (!IsTruthy(result))
        return false;
      cur = cell->cdr;
    }
    return result;
  }
  if (n == "or") {
    auto cur = args;
    while (std::holds_alternative<std::shared_ptr<Pair>>(cur)) {
      auto cell = std::get<std::shared_ptr<Pair>>(cur);
      Value result = EvalExpr(cell->car, env);
      if (IsTruthy(result))
        return result;
      cur = cell->cdr;
    }
    return false;
  }
  throw std::runtime_error("unknown special form: " + n);
}

Value Evaluator::EvalDefine(const Value& args, std::shared_ptr<Environment> env) {
  auto cell = std::get<std::shared_ptr<Pair>>(args);
  if (std::holds_alternative<std::shared_ptr<Pair>>(cell->car)) {
    auto func_pair = std::get<std::shared_ptr<Pair>>(cell->car);
    auto name = std::get<Symbol>(func_pair->car).name;
    auto lambda = std::make_shared<Lambda>();
    auto cur = func_pair->cdr;
    while (std::holds_alternative<std::shared_ptr<Pair>>(cur)) {
      auto p = std::get<std::shared_ptr<Pair>>(cur);
      lambda->params.push_back(std::get<Symbol>(p->car).name);
      cur = p->cdr;
    }
    auto body_cell = std::get<std::shared_ptr<Pair>>(cell->cdr);
    lambda->body = body_cell->car;
    lambda->closure = env;
    env->Define(name, lambda);
    return lambda;
  }
  auto name = std::get<Symbol>(cell->car).name;
  auto val_pair = std::get<std::shared_ptr<Pair>>(cell->cdr);
  auto val = EvalExpr(val_pair->car, env);
  env->Define(name, val);
  return val;
}

Value Evaluator::EvalIf(const Value& args, std::shared_ptr<Environment> env) {
  auto cell = std::get<std::shared_ptr<Pair>>(args);
  auto cond = EvalExpr(cell->car, env);
  auto rest = std::get<std::shared_ptr<Pair>>(cell->cdr);
  if (IsTruthy(cond))
    return EvalExpr(rest->car, env);
  auto else_part = std::get<std::shared_ptr<Pair>>(rest->cdr);
  return EvalExpr(else_part->car, env);
}

Value Evaluator::EvalCond(const Value& args, std::shared_ptr<Environment> env) {
  auto cur = args;
  while (std::holds_alternative<std::shared_ptr<Pair>>(cur)) {
    auto clause = std::get<std::shared_ptr<Pair>>(cur);
    auto cp = std::get<std::shared_ptr<Pair>>(clause->car);
    if (std::holds_alternative<Symbol>(cp->car) && std::get<Symbol>(cp->car).name == "else") {
      return EvalExpr(std::get<std::shared_ptr<Pair>>(cp->cdr)->car, env);
    }
    if (IsTruthy(EvalExpr(cp->car, env))) {
      return EvalExpr(std::get<std::shared_ptr<Pair>>(cp->cdr)->car, env);
    }
    cur = clause->cdr;
  }
  return Nil{};
}

Value Evaluator::EvalLet(const Value& args, std::shared_ptr<Environment> env) {
  auto cell = std::get<std::shared_ptr<Pair>>(args);
  auto bindings_val = cell->car;
  auto let_env = std::make_shared<Environment>(env);
  auto cur = bindings_val;
  while (std::holds_alternative<std::shared_ptr<Pair>>(cur)) {
    auto binding = std::get<std::shared_ptr<Pair>>(cur);
    auto b = std::get<std::shared_ptr<Pair>>(binding->car);
    auto bname = std::get<Symbol>(b->car).name;
    auto bval = EvalExpr(std::get<std::shared_ptr<Pair>>(b->cdr)->car, env);
    let_env->Define(bname, bval);
    cur = binding->cdr;
  }
  auto body = std::get<std::shared_ptr<Pair>>(cell->cdr);
  return EvalExpr(body->car, let_env);
}

Value Evaluator::EvalLambda(const Value& args, std::shared_ptr<Environment> env) {
  auto cell = std::get<std::shared_ptr<Pair>>(args);
  auto param_val = cell->car;
  auto body_cell = std::get<std::shared_ptr<Pair>>(cell->cdr);
  std::vector<std::string> params;
  auto cur = param_val;
  while (std::holds_alternative<std::shared_ptr<Pair>>(cur)) {
    auto p = std::get<std::shared_ptr<Pair>>(cur);
    params.push_back(std::get<Symbol>(p->car).name);
    cur = p->cdr;
  }
  auto lambda = std::make_shared<Lambda>();
  lambda->params = std::move(params);
  lambda->body = body_cell->car;
  lambda->closure = env;
  return lambda;
}