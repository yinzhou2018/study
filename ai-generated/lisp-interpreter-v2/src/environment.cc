#include "environment.h"

#include <stdexcept>

Environment::Environment(std::shared_ptr<Environment> parent) : parent_(std::move(parent)) {}

Value Environment::Lookup(const std::string& name) const {
  auto it = bindings_.find(name);
  if (it != bindings_.end())
    return it->second;
  if (parent_)
    return parent_->Lookup(name);
  throw std::runtime_error("unbound variable: " + name);
}

void Environment::Define(const std::string& name, const Value& value) {
  bindings_[name] = value;
}

bool Environment::Set(const std::string& name, const Value& value) {
  auto it = bindings_.find(name);
  if (it != bindings_.end()) {
    it->second = value;
    return true;
  }
  if (parent_)
    return parent_->Set(name, value);
  return false;
}
