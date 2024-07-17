#include "reflection_sample.h"
#include <algorithm>
#include <iostream>

#define RELECTION_TYPE_IMPL
#include "reflection_object.inl"

using namespace reflection;

ReflectionObject::ReflectionObject() {
  age_ = ObjectPtr(new IntObject);
  name_ = ObjectPtr(new StringObject);
}

ReflectionObject::~ReflectionObject() {}

ObjectPtr ReflectionObject::add_age(const ObjectPtrList& args) {
  auto a = dynamic_cast_object<IntObject>(args[0])->value_;
  auto b = dynamic_cast_object<IntObject>(args[1])->value_;
  auto& age = dynamic_cast_object<IntObject>(age_)->value_;
  age = add_age_impl(age, a + b);
  return age_;
}

ObjectPtr ReflectionObject::reanme(const ObjectPtrList& args) {
  name_ = args[0];
  return name_;
}

int ReflectionObject::add_age_impl(int base, int increment) {
  return base + increment;
}

void test_reflection_sample() {
  const auto& types = reflection::get_all_register_types();
  std::for_each(types.begin(), types.end(), [](const auto& type_pair) {
    std::cout << "type name: " << type_pair.first << ", parent type name : "
              << ((type_pair.second->parent != nullptr) ? type_pair.second->parent->name : "null") << std::endl;

    auto properties = type_pair.second->properties;
    if (properties) {
      std::for_each(properties->begin(), properties->end(), [](const auto& property) {
        std::cout << "property name: " << property.first << ", property type name: " << property.second.type->name
                  << std::endl;
      });
    }

    auto methods = type_pair.second->methods;
    if (methods) {
      std::for_each(methods->begin(), methods->end(), [](const auto& method) {
        std::cout << "method name: " << method.first << ", method return type name: " << method.second.return_type->name
                  << ", method parameter type name: ";
        std::for_each(method.second.parameter_types.begin(), method.second.parameter_types.end(),
                      [](const auto& parameter) { std::cout << parameter->name << ", "; });
        std::cout << std::endl;
      });
    }
  });

  auto object = reflection::create_object("ReflectionObject");
  std::cout << "object type name: " << object->get_type()->name << std::endl;
  auto int_object = new reflection::IntObject(100);
  object->set_property("age", reflection::ObjectPtr(int_object));
  auto age = object->get_property("age");
  std::cout << "age: " << dynamic_cast_object<IntObject>(age)->value_ << std::endl;
  object->invoke_method("add_age", {ObjectPtr(new IntObject(10)), ObjectPtr(new IntObject(20))});
  std::cout << "age: " << dynamic_cast_object<IntObject>(age)->value_ << std::endl;
  object->invoke_method("reanme", {ObjectPtr(new StringObject("hello"))});
  std::cout << "name: " << dynamic_cast_object<StringObject>(object->get_property("name"))->value_ << std::endl;
  object->set_property("name", ObjectPtr(new StringObject("world")));
  std::cout << "name: " << dynamic_cast_object<StringObject>(object->get_property("name"))->value_ << std::endl;
}