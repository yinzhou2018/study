#pragma once
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
using TypeMap = std::map<std::string, Type*>;
using PropertyMap = std::map<std::string, Type*>;
using MethodMap = std::map<std::string, std::pair<Type*, TypePtrList>>;
using PFCreateInstance = std::function<ObjectPtr()>;

struct Object {
  virtual ~Object() = default;
  virtual const Type* get_type() const { return s_type_; }
  virtual bool set_property(const std::string& name, const ObjectPtr value) { return false; }
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

template <typename TObject>
inline const TObject* dynamic_cast_object(const Object* object) {
  if (object->get_type() == TObject::s_type_) {
    return static_cast<const TObject*>(object);
  }
  return nullptr;
}

#define BEGINE_REFLECTION_TYPE(type_name, parent_type_name)                                                       \
  class type_name : public parent_type_name {                                                                     \
   public:                                                                                                        \
    type_name() = default;                                                                                        \
    ~type_name() override = default;                                                                              \
    const Type* get_type() const override {                                                                       \
      return s_type_;                                                                                             \
    }                                                                                                             \
    bool set_property(const std::string& name, const reflection::ObjectPtr value) override;                       \
    reflection::ObjectPtr get_property(const std::string& name) const override;                                   \
    reflection::ObjectPtr invoke_method(const std::string& name, const reflection::ObjectPtrList& args) override; \
    static Type* s_type_;                                                                                         \
    static PropertyMap s_properties_;                                                                             \
    static MethodMap s_methods_;

#define REFLECTION_PROPERTY(type, name) std::shared_ptr<type> name##_;

#define REFLECTION_METHOD(return_type, name) reflection::ObjectPtr name(const reflection::ObjectPtrList& args);

#define END_REFLECTION_TYPE() \
  }                           \
  ;

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
};

class FirstObject : public Object {
 public:
  ~FirstObject() override = default;
  const Type* get_type() const override { return s_type_; }

  bool set_property(const std::string& name, const ObjectPtr value) override {
    if (name == "age") {
      auto int_object = dynamic_cast_object<IntObject>(value.get());
      age_ = int_object->value_;
    } else if (name == "name") {
      auto string_object = dynamic_cast_object<StringObject>(value.get());
      name_ = string_object->value_;
    }
    return false;
  }

  ObjectPtr get_property(const std::string& name) const override {
    if (name == "age") {
      auto int_object = new IntObject;
      int_object->value_ = age_;
      return ObjectPtr(int_object);
    } else if (name == "name") {
      auto string_object = new StringObject;
      string_object->value_ = name_;
      return ObjectPtr(string_object);
    }
    return nullptr;
  }

  ObjectPtr invoke_method(const std::string& name, const ObjectPtrList& args) override {
    if (name == "add_age") {
      auto int1_object = dynamic_cast_object<IntObject>(args[0].get());
      auto int2_object = dynamic_cast_object<IntObject>(args[1].get());
      auto result = add_age(int1_object->value_, int2_object->value_);
      auto int_object = new IntObject;
      int_object->value_ = result;
      return ObjectPtr(int_object);
    } else if (name == "reanme") {
      auto string_object = dynamic_cast_object<StringObject>(args[0].get());
      auto result = reanme(string_object->value_);
      auto result_object = new StringObject;
      result_object->value_ = result;
      return ObjectPtr(result_object);
    }
    return nullptr;
  }

  int add_age(int a, int b) {
    age_ += (a + b);
    return age_;
  }

  std::string reanme(const std::string& name) {
    name_ = name;
    return name_;
  }

  int age_ = 0;
  std::string name_;

  static PropertyMap s_properties_;
  static MethodMap s_methods_;
  static Type* s_type_;
};

Type* Object::s_type_ = register_type("Object", nullptr, []() { return nullptr; }, nullptr, nullptr);
Type* IntObject::s_type_ = register_type("Int", nullptr, []() { return ObjectPtr(new IntObject); }, nullptr, nullptr);
Type* StringObject::s_type_ =
    register_type("String", nullptr, []() { return ObjectPtr(new StringObject); }, nullptr, nullptr);
Type* VoidObject::s_type_ =
    register_type("Void", nullptr, []() { return ObjectPtr(new VoidObject); }, nullptr, nullptr);

PropertyMap FirstObject::s_properties_ = {{"age", IntObject::s_type_}, {"name", StringObject::s_type_}};
MethodMap FirstObject::s_methods_ = {
    {"add_age", {IntObject::s_type_, {IntObject::s_type_, IntObject::s_type_}}},
    {"reanme", {StringObject::s_type_, {StringObject::s_type_}}},
};
Type* FirstObject::s_type_ = register_type(
    "FirstObject",
    Object::s_type_,
    []() { return ObjectPtr(new FirstObject); },
    &s_properties_,
    &s_methods_);

ObjectPtr create_object(const std::string& type_name) {
  if (g_types.find(type_name) == g_types.end()) {
    return nullptr;
  }
  return g_types[type_name]->create_instance();
}

}  // namespace reflection
