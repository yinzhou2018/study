#include "eval.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

// 创建全局环境
std::shared_ptr<Env> createGlobalEnv() {
  auto env = std::make_shared<Env>();

  // 算术运算
  env->define("+", std::make_shared<BuiltinFunc>("+", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                double sum = 0;
                for (const auto& a : args) {
                  if (!a->isNumber())
                    throw std::runtime_error("+ expects numbers");
                  sum += std::dynamic_pointer_cast<Number>(a)->get();
                }
                return std::make_shared<Number>(sum);
              }));

  env->define("-", std::make_shared<BuiltinFunc>("-", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                if (args.empty())
                  throw std::runtime_error("- needs at least one argument");
                if (!args[0]->isNumber())
                  throw std::runtime_error("- expects numbers");
                double result = std::dynamic_pointer_cast<Number>(args[0])->get();
                if (args.size() == 1)
                  return std::make_shared<Number>(-result);
                for (size_t i = 1; i < args.size(); ++i) {
                  if (!args[i]->isNumber())
                    throw std::runtime_error("- expects numbers");
                  result -= std::dynamic_pointer_cast<Number>(args[i])->get();
                }
                return std::make_shared<Number>(result);
              }));

  env->define("*", std::make_shared<BuiltinFunc>("*", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                double product = 1;
                for (const auto& a : args) {
                  if (!a->isNumber())
                    throw std::runtime_error("* expects numbers");
                  product *= std::dynamic_pointer_cast<Number>(a)->get();
                }
                return std::make_shared<Number>(product);
              }));

  env->define("/", std::make_shared<BuiltinFunc>("/", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                if (args.size() != 2)
                  throw std::runtime_error("/ needs exactly two arguments");
                if (!args[0]->isNumber() || !args[1]->isNumber())
                  throw std::runtime_error("/ expects numbers");
                double a = std::dynamic_pointer_cast<Number>(args[0])->get();
                double b = std::dynamic_pointer_cast<Number>(args[1])->get();
                if (b == 0)
                  throw std::runtime_error("Division by zero");
                return std::make_shared<Number>(a / b);
              }));

  // 比较运算
  env->define("<", std::make_shared<BuiltinFunc>("<", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                if (args.size() != 2)
                  throw std::runtime_error("< needs exactly two arguments");
                if (!args[0]->isNumber() || !args[1]->isNumber())
                  throw std::runtime_error("< expects numbers");
                return std::dynamic_pointer_cast<Number>(args[0])->get() <
                               std::dynamic_pointer_cast<Number>(args[1])->get()
                           ? std::make_shared<Symbol>("t")
                           : nil();
              }));

  env->define(">", std::make_shared<BuiltinFunc>(">", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                if (args.size() != 2)
                  throw std::runtime_error("> needs exactly two arguments");
                if (!args[0]->isNumber() || !args[1]->isNumber())
                  throw std::runtime_error("> expects numbers");
                return std::dynamic_pointer_cast<Number>(args[0])->get() >
                               std::dynamic_pointer_cast<Number>(args[1])->get()
                           ? std::make_shared<Symbol>("t")
                           : nil();
              }));

  env->define("=", std::make_shared<BuiltinFunc>("=", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                if (args.size() != 2)
                  throw std::runtime_error("= needs exactly two arguments");
                if (!args[0]->isNumber() || !args[1]->isNumber())
                  throw std::runtime_error("= expects numbers");
                return std::dynamic_pointer_cast<Number>(args[0])->get() ==
                               std::dynamic_pointer_cast<Number>(args[1])->get()
                           ? std::make_shared<Symbol>("t")
                           : nil();
              }));

  // 数学函数
  env->define("abs", std::make_shared<BuiltinFunc>("abs", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                if (args.size() != 1)
                  throw std::runtime_error("abs needs exactly one argument");
                if (!args[0]->isNumber())
                  throw std::runtime_error("abs expects a number");
                double val = std::dynamic_pointer_cast<Number>(args[0])->get();
                return std::make_shared<Number>(std::abs(val));
              }));

  // 列表操作
  env->define("cons",
              std::make_shared<BuiltinFunc>("cons", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                if (args.size() != 2)
                  throw std::runtime_error("cons needs exactly two arguments");
                std::vector<ValuePtr> result;
                result.push_back(args[0]);
                if (args[1]->isList() && !args[1]->isNil()) {
                  auto lst = std::dynamic_pointer_cast<List>(args[1]);
                  for (const auto& item : lst->get()) {
                    result.push_back(item);
                  }
                } else if (!args[1]->isNil()) {
                  throw std::runtime_error("cons second argument must be a list");
                }
                return std::make_shared<List>(result);
              }));

  env->define("car", std::make_shared<BuiltinFunc>("car", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                if (args.size() != 1)
                  throw std::runtime_error("car needs exactly one argument");
                if (args[0]->isNil())
                  throw std::runtime_error("car cannot operate on empty list");
                if (!args[0]->isList())
                  throw std::runtime_error("car expects a list");
                auto lst = std::dynamic_pointer_cast<List>(args[0]);
                return (*lst)[0];
              }));

  env->define("cdr", std::make_shared<BuiltinFunc>("cdr", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                if (args.size() != 1)
                  throw std::runtime_error("cdr needs exactly one argument");
                if (args[0]->isNil())
                  throw std::runtime_error("cdr cannot operate on empty list");
                if (!args[0]->isList())
                  throw std::runtime_error("cdr expects a list");
                auto lst = std::dynamic_pointer_cast<List>(args[0]);
                std::vector<ValuePtr> result;
                for (size_t i = 1; i < lst->size(); ++i) {
                  result.push_back((*lst)[i]);
                }
                return result.empty() ? nil() : std::make_shared<List>(result);
              }));

  env->define("list",
              std::make_shared<BuiltinFunc>("list", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                return std::make_shared<List>(args);
              }));

  // 输出
  env->define("print",
              std::make_shared<BuiltinFunc>("print", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) {
                for (size_t i = 0; i < args.size(); ++i) {
                  if (args[i]->isString()) {
                    auto s = std::dynamic_pointer_cast<String>(args[i]);
                    std::cout << s->get();
                  } else {
                    std::cout << args[i]->toString();
                  }
                  if (i + 1 < args.size())
                    std::cout << " ";
                }
                std::cout << std::endl;
                return nil();
              }));

  // null 检查
  env->define("null?", std::make_shared<BuiltinFunc>(
                           "null?", [](const std::vector<ValuePtr>& args, std::shared_ptr<Env>) -> ValuePtr {
                             if (args.size() != 1)
                               throw std::runtime_error("null? needs exactly one argument");
                             // 先检查是否为 nil
                             if (args[0]->isNil())
                               return std::make_shared<Symbol>("t");
                             // 再检查是否为空列表（Nil::isList() 也返回 true，但上面已经排除了）
                             if (args[0]->isList()) {
                               auto lst = std::dynamic_pointer_cast<List>(args[0]);
                               if (lst && lst->empty())
                                 return std::make_shared<Symbol>("t");
                             }
                             return nil();
                           }));

  return env;
}

// 求值函数
ValuePtr eval(ValuePtr expr, std::shared_ptr<Env> env) {
  // 数字和字符串直接返回
  if (expr->isNumber() || expr->isString()) {
    return expr;
  }

  // 符号查找环境
  if (expr->isSymbol()) {
    auto sym = std::dynamic_pointer_cast<Symbol>(expr);
    auto val = env->lookup(sym->get());
    if (!val) {
      throw std::runtime_error("Undefined symbol: " + sym->get());
    }
    return val;
  }

  // nil 直接返回
  if (expr->isNil()) {
    return expr;
  }

  // 列表处理
  if (expr->isList()) {
    auto lst = std::dynamic_pointer_cast<List>(expr);
    if (lst->empty()) {
      return nil();
    }

    // 第一个元素作为操作符
    auto first = (*lst)[0];
    if (!first->isSymbol()) {
      // 如果不是符号，先求值
      auto evaluated = eval(first, env);
      if (evaluated->isLambda()) {
        // 函数调用
        auto lambda = std::dynamic_pointer_cast<Lambda>(evaluated);
        std::vector<ValuePtr> args;
        for (size_t i = 1; i < lst->size(); ++i) {
          args.push_back(eval((*lst)[i], env));
        }
        // 创建新环境
        auto newEnv = std::make_shared<Env>(lambda->getClosure());
        const auto& params = lambda->getParams();
        if (args.size() != params.size()) {
          throw std::runtime_error("Wrong number of arguments to lambda");
        }
        for (size_t i = 0; i < params.size(); ++i) {
          newEnv->define(params[i], args[i]);
        }
        return eval(lambda->getBody(), newEnv);
      }
      throw std::runtime_error("Cannot apply non-function");
    }

    auto sym = std::dynamic_pointer_cast<Symbol>(first);
    const std::string& name = sym->get();

    // 特殊形式
    if (name == "quote") {
      if (lst->size() != 2)
        throw std::runtime_error("quote needs exactly one argument");
      return (*lst)[1];
    }

    if (name == "if") {
      if (lst->size() != 4)
        throw std::runtime_error("if needs exactly three arguments");
      auto cond = eval((*lst)[1], env);
      if (!cond->isNil()) {
        return eval((*lst)[2], env);
      } else {
        return eval((*lst)[3], env);
      }
    }

    if (name == "define") {
      if (lst->size() < 3)
        throw std::runtime_error("define needs at least two arguments");

      auto firstArg = (*lst)[1];

      // 检查是否是函数定义语法: (define (name args...) body...)
      if (firstArg->isList()) {
        auto sig = std::dynamic_pointer_cast<List>(firstArg);
        if (sig->empty())
          throw std::runtime_error("define function signature cannot be empty");

        auto funcNameSym = std::dynamic_pointer_cast<Symbol>((*sig)[0]);
        if (!funcNameSym)
          throw std::runtime_error("function name must be a symbol");

        // 提取参数名
        std::vector<std::string> paramNames;
        for (size_t i = 1; i < sig->size(); ++i) {
          auto pSym = std::dynamic_pointer_cast<Symbol>((*sig)[i]);
          if (!pSym)
            throw std::runtime_error("function parameters must be symbols");
          paramNames.push_back(pSym->get());
        }

        // 创建函数体：如果有多个表达式，用 begin 包裹
        ValuePtr body;
        if (lst->size() == 3) {
          body = (*lst)[2];
        } else {
          // 构建 (begin expr1 expr2 ...) 列表
          std::vector<ValuePtr> beginList;
          beginList.push_back(std::make_shared<Symbol>("begin"));
          for (size_t i = 2; i < lst->size(); ++i) {
            beginList.push_back((*lst)[i]);
          }
          body = std::make_shared<List>(beginList);
        }

        // 创建 lambda
        auto lambda = std::make_shared<Lambda>(paramNames, body, env);
        env->define(funcNameSym->get(), lambda);
        return lambda;
      }

      // 普通变量定义: (define symbol value)
      if (lst->size() != 3)
        throw std::runtime_error("define variable needs exactly two arguments");
      auto varSym = std::dynamic_pointer_cast<Symbol>(firstArg);
      if (!varSym)
        throw std::runtime_error("define first argument must be a symbol or function signature");
      auto val = eval((*lst)[2], env);
      env->define(varSym->get(), val);
      return val;
    }

    if (name == "lambda") {
      if (lst->size() != 3)
        throw std::runtime_error("lambda needs exactly two arguments");
      auto params = std::dynamic_pointer_cast<List>((*lst)[1]);
      if (!params)
        throw std::runtime_error("lambda first argument must be a parameter list");
      std::vector<std::string> paramNames;
      for (const auto& p : params->get()) {
        auto pSym = std::dynamic_pointer_cast<Symbol>(p);
        if (!pSym)
          throw std::runtime_error("lambda parameters must be symbols");
        paramNames.push_back(pSym->get());
      }
      return std::make_shared<Lambda>(paramNames, (*lst)[2], env);
    }

    if (name == "begin") {
      ValuePtr result = nil();
      for (size_t i = 1; i < lst->size(); ++i) {
        result = eval((*lst)[i], env);
      }
      return result;
    }

    // 普通函数调用
    auto func = env->lookup(name);
    if (!func) {
      throw std::runtime_error("Undefined function: " + name);
    }

    // 求值所有参数
    std::vector<ValuePtr> args;
    for (size_t i = 1; i < lst->size(); ++i) {
      args.push_back(eval((*lst)[i], env));
    }

    // 调用内置函数
    if (auto builtin = std::dynamic_pointer_cast<BuiltinFunc>(func)) {
      return builtin->call(args, env);
    }

    // 调用 lambda
    if (auto lambda = std::dynamic_pointer_cast<Lambda>(func)) {
      auto newEnv = std::make_shared<Env>(lambda->getClosure());
      const auto& params = lambda->getParams();
      if (args.size() != params.size()) {
        throw std::runtime_error("Wrong number of arguments to " + name);
      }
      for (size_t i = 0; i < params.size(); ++i) {
        newEnv->define(params[i], args[i]);
      }
      return eval(lambda->getBody(), newEnv);
    }

    throw std::runtime_error("Cannot apply: " + name);
  }

  throw std::runtime_error("Unknown expression type");
}