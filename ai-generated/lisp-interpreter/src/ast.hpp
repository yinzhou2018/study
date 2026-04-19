#ifndef AST_HPP
#define AST_HPP

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// 前向声明
class Env;

// Value 基类 - 所有 Lisp 值的基类
class Value {
 public:
  virtual ~Value() = default;
  virtual std::string toString() const = 0;
  virtual bool isNumber() const { return false; }
  virtual bool isString() const { return false; }
  virtual bool isSymbol() const { return false; }
  virtual bool isList() const { return false; }
  virtual bool isLambda() const { return false; }
  virtual bool isNil() const { return false; }
};

using ValuePtr = std::shared_ptr<Value>;

// Number - 数值类型
class Number : public Value {
  double value;

 public:
  explicit Number(double v) : value(v) {}
  double get() const { return value; }
  std::string toString() const override {
    if (value == static_cast<long long>(value)) {
      return std::to_string(static_cast<long long>(value));
    }
    return std::to_string(value);
  }
  bool isNumber() const override { return true; }
};

// String - 字符串类型
class String : public Value {
  std::string value;

 public:
  explicit String(const std::string& v) : value(v) {}
  const std::string& get() const { return value; }
  std::string toString() const override { return "\"" + value + "\""; }
  bool isString() const override { return true; }
};

// Symbol - 符号类型
class Symbol : public Value {
  std::string name;

 public:
  explicit Symbol(const std::string& n) : name(n) {}
  const std::string& get() const { return name; }
  std::string toString() const override { return name; }
  bool isSymbol() const override { return true; }
};

// List - 列表类型
class List : public Value {
  std::vector<ValuePtr> items;

 public:
  List() = default;
  explicit List(std::vector<ValuePtr> i) : items(std::move(i)) {}

  const std::vector<ValuePtr>& get() const { return items; }
  size_t size() const { return items.size(); }
  bool empty() const { return items.empty(); }
  ValuePtr operator[](size_t i) const { return items[i]; }

  std::string toString() const override {
    std::string result = "(";
    for (size_t i = 0; i < items.size(); ++i) {
      if (i > 0)
        result += " ";
      result += items[i]->toString();
    }
    result += ")";
    return result;
  }
  bool isList() const override { return true; }
};

// Nil - 空列表
class Nil : public Value {
 public:
  std::string toString() const override { return "nil"; }
  bool isNil() const override { return true; }
  bool isList() const override { return true; }
};

// Lambda - lambda 表达式
class Lambda : public Value {
  std::vector<std::string> params;
  ValuePtr body;
  std::shared_ptr<Env> closure;

 public:
  Lambda(std::vector<std::string> p, ValuePtr b, std::shared_ptr<Env> c)
      : params(std::move(p)), body(std::move(b)), closure(std::move(c)) {}

  const std::vector<std::string>& getParams() const { return params; }
  ValuePtr getBody() const { return body; }
  std::shared_ptr<Env> getClosure() const { return closure; }

  std::string toString() const override { return "<lambda>"; }
  bool isLambda() const override { return true; }
};

// BuiltinFunc - 内置函数
class BuiltinFunc : public Value {
  std::string name;
  std::function<ValuePtr(const std::vector<ValuePtr>&, std::shared_ptr<Env>)> func;

 public:
  BuiltinFunc(std::string n, std::function<ValuePtr(const std::vector<ValuePtr>&, std::shared_ptr<Env>)> f)
      : name(std::move(n)), func(std::move(f)) {}

  ValuePtr call(const std::vector<ValuePtr>& args, std::shared_ptr<Env> env) { return func(args, env); }

  std::string toString() const override { return "<builtin:" + name + ">"; }
};

// Environment - 环境用于变量绑定
class Env {
  std::unordered_map<std::string, ValuePtr> bindings;
  std::shared_ptr<Env> parent;

 public:
  Env(std::shared_ptr<Env> p = nullptr) : parent(p) {}

  void define(const std::string& name, ValuePtr value) { bindings[name] = std::move(value); }

  ValuePtr lookup(const std::string& name) const {
    auto it = bindings.find(name);
    if (it != bindings.end()) {
      return it->second;
    }
    if (parent) {
      return parent->lookup(name);
    }
    return nullptr;
  }

  bool set(const std::string& name, ValuePtr value) {
    auto it = bindings.find(name);
    if (it != bindings.end()) {
      bindings[name] = std::move(value);
      return true;
    }
    if (parent) {
      return parent->set(name, value);
    }
    return false;
  }
};

// 全局 Nil 单例
inline ValuePtr nil() {
  static ValuePtr instance = std::make_shared<Nil>();
  return instance;
}

#endif  // AST_HPP