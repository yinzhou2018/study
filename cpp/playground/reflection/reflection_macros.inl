// 需要反射的类型借助下面的宏来自动暴露元数据，以及提供统一的属性及方法调用接口
// 这里参照Chromium代码技巧，同样的宏接口在不同条件宏下分别定义声明及实现，最大化减少代码重复。
#ifndef RELECTION_TYPE_IMPL
#define BEGIN_REFLECTION_TYPE(type_name, parent_type_name)                                                        \
  class type_name : public parent_type_name {                                                                     \
   public:                                                                                                        \
    type_name();                                                                                                  \
    ~type_name() override;                                                                                        \
    const reflection::Type* get_type() const override {                                                           \
      return s_type_;                                                                                             \
    }                                                                                                             \
    bool set_property(const std::string& name, reflection::ObjectPtr value) override;                             \
    reflection::ObjectPtr get_property(const std::string& name) const override;                                   \
    reflection::ObjectPtr invoke_method(const std::string& name, const reflection::ObjectPtrList& args) override; \
    static reflection::Type* s_type_;                                                                             \
    static reflection::PropertyMap s_properties_;                                                                 \
    static reflection::MethodMap s_methods_;                                                                      \
    using PFSetProp = void (*)(type_name*, const reflection::ObjectPtr);                                          \
    using PFGetProp = reflection::ObjectPtr (*)(const type_name*);                                                \
    using PFMethod = reflection::ObjectPtr (*)(type_name*, const reflection::ObjectPtrList&);

#define BEGIN_REFLECTION_PROPERTY(type_name)
#define REFLECTION_PROPERTY(type_name, name, prop_type)                         \
  reflection::ObjectPtr name##_;                                                \
  void set_##name(const reflection::ObjectPtr value) {                          \
    name##_->set_new_value(value);                                              \
  }                                                                             \
  reflection::ObjectPtr get_##name() const {                                    \
    return name##_;                                                             \
  }                                                                             \
  static void s_set_##name(type_name* obj, const reflection::ObjectPtr value) { \
    obj->set_##name(value);                                                     \
  }                                                                             \
  static reflection::ObjectPtr s_get_##name(const type_name* obj) {             \
    return obj->get_##name();                                                   \
  }
#define END_REFLECTION_PROPERTY()

#define BEGIN_REFLECTION_METHOD(type_name)
#define REFLECTION_METHOD(type_name, name, return_type)                                          \
  reflection::ObjectPtr name(const reflection::ObjectPtrList& args);                             \
  static reflection::ObjectPtr s_##name(type_name* obj, const reflection::ObjectPtrList& args) { \
    return obj->name(args);                                                                      \
  }
#define REFLECTION_METHOD_1(type_name, name, return_type, arg_type_1)                            \
  reflection::ObjectPtr name(const reflection::ObjectPtrList& args);                             \
  static reflection::ObjectPtr s_##name(type_name* obj, const reflection::ObjectPtrList& args) { \
    return obj->name(args);                                                                      \
  }
#define REFLECTION_METHOD_2(type_name, name, return_type, arg_type_1, arg_type_2)                \
  reflection::ObjectPtr name(const reflection::ObjectPtrList& args);                             \
  static reflection::ObjectPtr s_##name(type_name* obj, const reflection::ObjectPtrList& args) { \
    return obj->name(args);                                                                      \
  }
#define REFLECTION_METHOD_3(type_name, name, return_type, arg_type_1, arg_type_2, arg_type_3)    \
  reflection::ObjectPtr name(const reflection::ObjectPtrList& args);                             \
  static reflection::ObjectPtr s_##name(type_name* obj, const reflection::ObjectPtrList& args) { \
    return obj->name(args);                                                                      \
  }
#define END_REFLECTION_METHOD()

#define END_REFLECTION_TYPE() \
  }                           \
  ;

#else
#undef BEGIN_REFLECTION_TYPE
#undef BEGIN_REFLECTION_PROPERTY
#undef REFLECTION_PROPERTY
#undef END_REFLECTION_PROPERTY
#undef BEGIN_REFLECTION_METHOD
#undef REFLECTION_METHOD
#undef REFLECTION_METHOD_1
#undef REFLECTION_METHOD_2
#undef REFLECTION_METHOD_3
#undef END_REFLECTION_METHOD
#undef END_REFLECTION_TYPE

#define BEGIN_REFLECTION_TYPE(type_name, parent_type_name)                                                          \
  reflection::Type* type_name::s_type_ = reflection::register_type(                                                 \
      #type_name, reflection::get_register_type(#parent_type_name),                                                 \
      []() { return reflection::ObjectPtr(new type_name); }, &type_name::s_properties_, &type_name::s_methods_);    \
  bool type_name::set_property(const std::string& name, reflection::ObjectPtr value) {                              \
    auto it = s_properties_.find(name);                                                                             \
    if (it != s_properties_.end()) {                                                                                \
      if (!reflection::types_match(std::span(&it->second.type, 1), std::span(&value, 1))) {                         \
        assert(false);                                                                                              \
        return false;                                                                                               \
      }                                                                                                             \
      auto pf_set = reinterpret_cast<PFSetProp>(it->second.pf_set);                                                 \
      pf_set(this, value);                                                                                          \
      return true;                                                                                                  \
    }                                                                                                               \
    return parent_type_name::set_property(name, value);                                                             \
  }                                                                                                                 \
  reflection::ObjectPtr type_name::get_property(const std::string& name) const {                                    \
    auto it = s_properties_.find(name);                                                                             \
    if (it != s_properties_.end()) {                                                                                \
      auto pf_get = reinterpret_cast<PFGetProp>(it->second.pf_get);                                                 \
      return pf_get(this);                                                                                          \
    }                                                                                                               \
    return parent_type_name::get_property(name);                                                                    \
  }                                                                                                                 \
  reflection::ObjectPtr type_name::invoke_method(const std::string& name, const reflection::ObjectPtrList& args) {  \
    auto it = s_methods_.find(name);                                                                                \
    if (it != s_methods_.end()) {                                                                                   \
      if (!reflection::types_match(std::span(it->second.parameter_types.begin(), it->second.parameter_types.end()), \
                                   std::span(args.begin(), args.end()))) {                                          \
        assert(false);                                                                                              \
        return nullptr;                                                                                             \
      }                                                                                                             \
      auto pf_method = reinterpret_cast<PFMethod>(it->second.pf_method);                                            \
      return pf_method(this, args);                                                                                 \
    }                                                                                                               \
    return parent_type_name::invoke_method(name, args);                                                             \
  }

#define BEGIN_REFLECTION_PROPERTY(type_name) reflection::PropertyMap type_name::s_properties_ = {
#define REFLECTION_PROPERTY(type_name, name, prop_type) \
  {#name, {prop_type::s_type_, (void*)&type_name::s_set_##name, (void*)&type_name::s_get_##name}},
#define END_REFLECTION_PROPERTY() \
  }                               \
  ;

#define BEGIN_REFLECTION_METHOD(type_name) reflection::MethodMap type_name::s_methods_ = {
#define REFLECTION_METHOD(type_name, name, return_type) \
  {#name, {return_type::s_type_, {}, (void*)&type_name::s_##name}},
#define REFLECTION_METHOD_1(type_name, name, return_type, arg_type_1) \
  {#name, {return_type::s_type_, {arg_type_1::s_type_}, (void*)&type_name::s_##name}},
#define REFLECTION_METHOD_2(type_name, name, return_type, arg_type_1, arg_type_2) \
  {#name, {return_type::s_type_, {arg_type_1::s_type_, arg_type_2::s_type_}, (void*)&type_name::s_##name}},
#define REFLECTION_METHOD_3(type_name, name, return_type, arg_type_1, arg_type_2, arg_type_3) \
  {#name,                                                                                     \
   {return_type::s_type_,                                                                     \
    {arg_type_1::s_type_, arg_type_2::s_type_, arg_type_3::s_type_},                          \
    (void*)&type_name::s_##name}},
#define END_REFLECTION_METHOD() \
  }                             \
  ;

#define END_REFLECTION_TYPE()
#endif  // RELECTION_TYPE_IMPL