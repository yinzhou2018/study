#include "reflection_sample.h"

// PropertyMap FirstObject::s_properties_ = {{"age", IntObject::s_type_}, {"name", StringObject::s_type_}};
// MethodMap FirstObject::s_methods_ = {
//     {"add_age", {IntObject::s_type_, {IntObject::s_type_, IntObject::s_type_}}},
//     {"reanme", {StringObject::s_type_, {StringObject::s_type_}}},
// };
// Type* FirstObject::s_type_ = register_type(
//     "FirstObject",
//     Object::s_type_,
//     []() { return ObjectPtr(new FirstObject); },
//     &s_properties_,
//     &s_methods_);

// bool FirstObject::set_property(const std::string& name, const ObjectPtr value) {
//   if (name == "age") {
//     auto int_object = dynamic_cast_object<IntObject>(value.get());
//     age_ = int_object->value_;
//   } else if (name == "name") {
//     auto string_object = dynamic_cast_object<StringObject>(value.get());
//     name_ = string_object->value_;
//   }
//   return false;
// }

// ObjectPtr FirstObject::get_property(const std::string& name) const {
//   if (name == "age") {
//     auto int_object = new IntObject;
//     int_object->value_ = age_;
//     return ObjectPtr(int_object);
//   } else if (name == "name") {
//     auto string_object = new StringObject;
//     string_object->value_ = name_;
//     return ObjectPtr(string_object);
//   }
//   return nullptr;
// }

// ObjectPtr FirstObject::invoke_method(const std::string& name, const ObjectPtrList& args) {
//   if (name == "add_age") {
//     auto int1_object = dynamic_cast_object<IntObject>(args[0].get());
//     auto int2_object = dynamic_cast_object<IntObject>(args[1].get());
//     auto result = add_age(int1_object->value_, int2_object->value_);
//     auto int_object = new IntObject;
//     int_object->value_ = result;
//     return ObjectPtr(int_object);
//   } else if (name == "reanme") {
//     auto string_object = dynamic_cast_object<StringObject>(args[0].get());
//     auto result = reanme(string_object->value_);
//     auto result_object = new StringObject;
//     result_object->value_ = result;
//     return ObjectPtr(result_object);
//   }
//   return nullptr;
// }