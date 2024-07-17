#include "reflection.h"

#include <iostream>
namespace reflection {

static TypeMap g_types;

Type* register_type(const std::string& type_name,
                    Type* parent,
                    PFCreateInstance create_instance,
                    PropertyMap* perperties,
                    MethodMap* methods) {
  std::cout << "register_type: " << type_name << std::endl;
  auto type = new Type{type_name, parent, create_instance, perperties, methods};
  g_types[type_name] = type;
  return type;
}

Type* get_register_type(const std::string& type_name) {
  auto it = g_types.find(type_name);
  return it != g_types.end() ? it->second : nullptr;
}

const TypeMap& get_all_register_types() {
  return g_types;
}

bool is_subclass_of(const Type* child, const Type* parent) {
  return child->parent == parent;
}

bool is_ancestor_of(const Type* ancestor, const Type* child) {
  auto parent = child->parent;
  while (parent) {
    if (parent == ancestor) {
      return true;
    }
    parent = parent->parent;
  }
  return false;
}

bool types_match(const std::span<Type*>& expected, const std::span<const ObjectPtr>& inputs) {
  if (expected.size() != inputs.size()) {
    return false;
  }
  for (size_t i = 0; i < expected.size(); ++i) {
    auto input_type = inputs[i]->get_type();
    if (input_type != expected[i] && !reflection::is_ancestor_of(expected[i], input_type)) {
      return false;
    }
  }
  return true;
}

ObjectPtr create_object(const std::string& type_name) {
  if (g_types.find(type_name) == g_types.end()) {
    return nullptr;
  }
  return g_types[type_name]->create_instance();
}

Type* Object::s_type_ = register_type("reflection::Object", nullptr, []() { return nullptr; }, nullptr, nullptr);
Type* IntObject::s_type_ =
    register_type("reflection::IntObject", nullptr, []() { return ObjectPtr(new IntObject); }, nullptr, nullptr);
Type* StringObject::s_type_ =
    register_type("reflection::StringObject", nullptr, []() { return ObjectPtr(new StringObject); }, nullptr, nullptr);
Type* VoidObject::s_type_ =
    register_type("reflection::VoidObject", nullptr, []() { return ObjectPtr(new VoidObject); }, nullptr, nullptr);

}  // namespace reflection