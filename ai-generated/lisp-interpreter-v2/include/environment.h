#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "value.h"

class Environment {
 public:
  explicit Environment(std::shared_ptr<Environment> parent = nullptr);

  Value Lookup(const std::string& name) const;
  void Define(const std::string& name, const Value& value);
  bool Set(const std::string& name, const Value& value);

 private:
  std::unordered_map<std::string, Value> bindings_;
  std::shared_ptr<Environment> parent_;
};

#endif  // ENVIRONMENT_H_
