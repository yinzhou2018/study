#include "reflection.h"

using namespace reflection;

class FirstObject : public Object {
 public:
  ~FirstObject() override = default;
  const Type* get_type() const override { return s_type_; }
  bool set_property(const std::string& name, const ObjectPtr value) override;
  ObjectPtr get_property(const std::string& name) const override;
  ObjectPtr invoke_method(const std::string& name, const ObjectPtrList& args) override;

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
};  // FirstObject