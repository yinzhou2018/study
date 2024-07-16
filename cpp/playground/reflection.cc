#include "reflection.h"

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

ObjectPtr create_object(const std::string& type_name) {
  if (g_types.find(type_name) == g_types.end()) {
    return nullptr;
  }
  return g_types[type_name]->create_instance();
}

Type* Object::s_type_ = register_type("Object", nullptr, []() { return nullptr; }, nullptr, nullptr);
Type* IntObject::s_type_ = register_type("Int", nullptr, []() { return ObjectPtr(new IntObject); }, nullptr, nullptr);
Type* StringObject::s_type_ =
    register_type("String", nullptr, []() { return ObjectPtr(new StringObject); }, nullptr, nullptr);
Type* VoidObject::s_type_ =
    register_type("Void", nullptr, []() { return ObjectPtr(new VoidObject); }, nullptr, nullptr);

}