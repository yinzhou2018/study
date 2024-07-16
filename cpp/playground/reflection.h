#pragma once
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <string>

// C++反射实现
namespace reflection {

struct Type;
struct Object;

using ObjectPtr = std::shared_ptr<Object>;
using ObjectPtrList = std::vector<ObjectPtr>;
using TypePtrList = std::vector<Type*>;
struct Property {
  Type* type;
  void* pf_set;
  void* pf_get;
};  // Property

struct Method {
  Type* return_type;
  TypePtrList parameter_types;
  void* pf_method;
};  // Method

using TypeMap = std::map<std::string, Type*>;
using PropertyMap = std::map<std::string, Property>;
using MethodMap = std::map<std::string, Method>;
using PFCreateInstance = std::function<ObjectPtr()>;
struct Object {
  virtual ~Object() = default;
  virtual const Type* get_type() const { return s_type_; }
  virtual bool set_property(const std::string& name, ObjectPtr value) { return false; }
  virtual ObjectPtr get_property(const std::string& name) const { return nullptr; }
  virtual ObjectPtr invoke_method(const std::string& name, const ObjectPtrList& args) { return nullptr; }
  static Type* s_type_;
};  // Object

struct Type {
  std::string name;
  Type* parent = nullptr;
  PFCreateInstance create_instance = nullptr;
  PropertyMap* properties;
  MethodMap* methods;
};  // Type

Type* register_type(const std::string& type_name,
                    Type* parent,
                    PFCreateInstance create_instance,
                    PropertyMap* perperties,
                    MethodMap* methods);
Type* get_register_type(const std::string& type_name);
const TypeMap& get_all_register_types();
ObjectPtr create_object(const std::string& type_name);

template <typename TObject>
inline TObject* dynamic_cast_object(Object* object) {
  if (object->get_type() == TObject::s_type_) {
    return static_cast<TObject*>(object);
  }
  return nullptr;
}

#ifndef RELECTION_TYPE_IMPL
#define BEGINE_REFLECTION_TYPE(type_name, parent_type_name)                                                       \
  class type_name : public parent_type_name {                                                                     \
   public:                                                                                                        \
    type_name() = default;                                                                                        \
    ~type_name() override = default;                                                                              \
    const Type* get_type() const override {                                                                       \
      return s_type_;                                                                                             \
    }                                                                                                             \
    bool set_property(const std::string& name, reflection::ObjectPtr value) override;                             \
    reflection::ObjectPtr get_property(const std::string& name) const override;                                   \
    reflection::ObjectPtr invoke_method(const std::string& name, const reflection::ObjectPtrList& args) override; \
    static Type* s_type_;                                                                                         \
    static PropertyMap s_properties_;                                                                             \
    static MethodMap s_methods_;                                                                                  \
    using PFSetProp = void (type_name::*)(const reflection::ObjectPtr);                                           \
    using PFGetProp = reflection::ObjectPtr (type_name::*)() const;                                               \
    using PFMethod = reflection::ObjectPtr (type_name::*)(const reflection::ObjectPtrList&);

#define BEGIN_REFLECTION_PROPERTY(type_name)
#define REFLECTION_PROPERTY(type_name, name, type)     \
  reflection::ObjectPtr name##_;                       \
  void set_##name(const reflection::ObjectPtr value) { \
    name##_ = value;                                   \
  }                                                    \
  reflection::ObjectPtr get_##name() const {           \
    return name##_;                                    \
  }
#define END_REFLECTION_PROPERTY()

#define BEGIN_REFLECTION_METHOD(type_name)
#define REFLECTION_METHOD(type_name, name, return_type) \
  reflection::ObjectPtr name(const reflection::ObjectPtrList& args);
#define REFLECTION_METHOD_1(type_name, name, return_type, arg_type_1) \
  reflection::ObjectPtr name(const reflection::ObjectPtrList& args);
#define REFLECTION_METHOD_2(type_name, name, return_type, arg_type_1, arg_type_2) \
  reflection::ObjectPtr name(const reflection::ObjectPtrList& args);
#define REFLECTION_METHOD_3(type_name, name, return_type, arg_type_1, arg_type_2, arg_type_3) \
  reflection::ObjectPtr name(const reflection::ObjectPtrList& args);

#define END_REFLECTION_METHOD()

#define END_REFLECTION_TYPE() \
  }                           \
  ;
#else
#define BEGINE_REFLECTION_TYPE(type_name, parent_type_name)         \
  Type* type_name::s_type_ = reflection::register_type(             \
      #type_name, reflection::get_register_type(#parent_type_name), \
      []() { return reflection::ObjectPtr(new type_name) }, &type_name::s_properties_, &type_name::s_methods_);

#define BEGIN_REFLECTION_PROPERTY(type_name) PropertyMap type_name::s_properties_ = {
#define REFLECTION_PROPERTY(type_name, name, type) \
  {#name, {type::s_type_, &type_name::set_##name, &type_name::get_##name}},
#define END_REFLECTION_PROPERTY() \
  }                               \
  ;

#define BEGIN_REFLECTION_METHOD(type_name) MethodMap type_name::s_methods_ = {
#define REFLECTION_METHOD(type_name, name, return_type) {#name, {return_type::s_type_, {}, &type_name::name}},
#define REFLECTION_METHOD_1(type_name, name, return_type, arg_type_1) \
  {#name, {return_type::s_type_, {arg_type_1::s_type_}, &type_name::name}},
#define REFLECTION_METHOD_2(type_name, name, return_type, arg_type_1, arg_type_2) \
  {#name, {return_type::s_type_, {arg_type_1::s_type_, arg_type_2::s_type_}, &type_name::name}},
#define REFLECTION_METHOD_3(type_name, name, return_type, arg_type_1, arg_type_2, arg_type_3) \
  {#name, {return_type::s_type_, {arg_type_1::s_type_, arg_type_2::s_type_, arg_type_3::s_type_}, &type_name::name}},
#define END_REFLECTION_METHOD() \
  }                             \
  ;

#endif  // RELECTION_TYPE_IMPL

class IntObject : public Object {
 public:
  ~IntObject() override = default;
  const Type* get_type() const override { return s_type_; }

  int value_ = 0;
  static Type* s_type_;
};

class VoidObject : public Object {
 public:
  ~VoidObject() override = default;
  const Type* get_type() const override { return s_type_; }
  static Type* s_type_;
};

class StringObject : public Object {
 public:
  ~StringObject() override = default;
  const Type* get_type() const override { return s_type_; }

  std::string value_;
  static Type* s_type_;
};  // StringObject

}  // namespace reflection
