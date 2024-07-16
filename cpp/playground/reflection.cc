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

}