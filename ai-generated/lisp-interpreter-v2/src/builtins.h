#ifndef BUILTINS_H_
#define BUILTINS_H_

#include <string>
#include <vector>

#include "value.h"

Value BuiltinArith(const std::string& op, const Value& args);
Value BuiltinListOp(const std::string& op, const Value& args);
Value BuiltinCompare(const std::string& op, const Value& args);
Value BuiltinBoolean(const std::string& op, const Value& args);

#endif  // BUILTINS_H_
