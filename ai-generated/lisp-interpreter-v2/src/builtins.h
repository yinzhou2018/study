#ifndef BUILTINS_H_
#define BUILTINS_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "value.h"

class Environment;

Value BuiltinArith(const std::string& op, const Value& args);
Value BuiltinListOp(const std::string& op, const Value& args);
Value BuiltinCompare(const std::string& op, const Value& args);
Value BuiltinBoolean(const std::string& op, const Value& args);

class BuiltinManager {
 public:
  static bool IsBuiltin(const std::string& name);
  static Value Call(const std::string& name, const Value& args);
  static void RegisterAll(std::shared_ptr<Environment> env);

 private:
  using Handler = std::function<Value(const std::string&, const Value&)>;

  static void InitRegistry();
  static void Register(const std::string& name, Handler handler);

  static std::unordered_map<std::string, Handler> handlers_;
};

#endif  // BUILTINS_H_
