#pragma once
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <vector>

// 如下为简单C++反射实现的基础设施, 基本原理：
// 1. 所有可反射类型都需要继承自`Object`, 并可选实现`get_type`, `set_property`, `get_property`,
// `invoke_method`方便动态创建及调用
// 2. 利用一些核心数据结构(`Type`, `Property`, `Method`)来记录类型元信息，方便运行时查询
// 3.基于类静态变量方式定义元信息示例并注册到全局类型表，利用了C++里类静态变量初始化先与main入口特性，保证了类型注册的时机
// 4.借助一些宏来自动实现上面的过程，最大化减少一些机械式的重复劳动，宏定义见`reflection_macros.inl`
// 局限性：
// 1.完全基于C++宏的方式定义反射类型相对有些晦涩，不利于阅读，同时也带来了下面的问题，更好的方式可能是扩展C++语法
// 2.反射类型只能单继承`Object`或`Object`的子类，多继承不支持
// 3.反射类型的属性及方法参数类型必须是`Object`的子类，不支持原生类型，实际使用中会有额外的装箱/拆箱操作，带来额外的性能问题
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
  virtual bool set_property(const std::string& name, ObjectPtr value) {
    assert(false);
    return false;
  }
  virtual ObjectPtr get_property(const std::string& name) const {
    assert(false);
    return nullptr;
  }
  virtual ObjectPtr invoke_method(const std::string& name, const ObjectPtrList& args) {
    assert(false);
    return nullptr;
  }

  virtual void set_new_value(ObjectPtr new_value) { assert(false); }

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
bool is_subclass_of(const Type* child, const Type* parent);
bool is_ancestor_of(const Type* ancestor, const Type* child);
ObjectPtr create_object(const std::string& type_name);
bool types_match(const std::span<Type*>& expected, const std::span<const ObjectPtr>& inputs);

template <typename TObject>
inline TObject* dynamic_cast_object(Object* object) {
  if (object->get_type() == TObject::s_type_) {
    return static_cast<TObject*>(object);
  }
  assert(false);
  return nullptr;
}

template <typename TObject>
inline TObject* dynamic_cast_object(ObjectPtr object) {
  return dynamic_cast_object<TObject>(object.get());
}

class IntObject : public Object {
 public:
  IntObject() = default;
  explicit IntObject(int value) : value_(value) {}
  ~IntObject() override = default;
  const Type* get_type() const override { return s_type_; }
  void set_new_value(ObjectPtr new_value) override {
    auto* new_value_int = dynamic_cast_object<IntObject>(new_value);
    value_ = new_value_int->value_;
  }

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
  StringObject() = default;
  explicit StringObject(const std::string_view& value) : value_(value) {}
  ~StringObject() override = default;
  const Type* get_type() const override { return s_type_; }
  void set_new_value(ObjectPtr new_value) override {
    auto* new_value_string = dynamic_cast_object<StringObject>(new_value);
    value_ = new_value_string->value_;
  }

  std::string value_;
  static Type* s_type_;
};  // StringObject

}  // namespace reflection
