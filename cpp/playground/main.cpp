#include <iostream>
#include "advanced_feature_study.h"
#include "coroutine_study.h"
#include "exception_study.h"
#include "multithread.h"
#include "preload_file.h"
#include "reflection_sample.h"
#include "shared_ptr_study.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
void core_foundation_study() {
  CFStringRef myString = CFStringCreateWithCString(NULL, "你好", kCFStringEncodingUTF8);

  char buffer[100] = {0};

  CFStringGetCString(myString, buffer, sizeof(buffer), kCFStringEncodingUTF8);

  // if (cString) {
  std::cout << buffer << std::endl;
  // }

  CFRelease(myString);
}
#endif  // __APPLE__

#define DEF_STRUCT(name, member1, member2) \
  struct name {                            \
    name() : member1##1(0), member2(0) {}  \
    int member1##1;                        \
    int member2;                           \
  };

DEF_STRUCT(TestStruct, age, height)

struct TestStruct2 : public TestStruct {
  std::string name;
};

int main(int, char**) {
  std::shared_ptr<TestStruct2> test_struct = std::make_shared<TestStruct2>();
  std::shared_ptr<TestStruct> test_struct2 = test_struct;
  std::cout << "test_struct2.use_count: " << test_struct2.use_count() << std::endl;
  // std::shared_ptr<TestStruct2> test_struct3 = test_struct2;

  std::cout << "Hello, Welcome to playground...\n";
  // advanced_feature_study();
  // preload_file(R"(E:\virtual_human_code\ue\virtual_human_ue51\Saved\1.0.1\Windows\VirtualHumanUE51\Content\Paks\base.pak)");
  // multithread_study();
  // shared_ptr_study();
  // core_foundation_study();
  // const auto& types = reflection::get_all_register_types();
  // std::for_each(types.begin(), types.end(), [](const auto& type_pair) {
  //   std::cout << "type name: " << type_pair.first << ", parent type name : "
  //             << ((type_pair.second->parent != nullptr) ? type_pair.second->parent->name : "null") << std::endl;
  //   auto properties = type_pair.second->properties;
  //   if (properties) {
  //     std::for_each(properties->begin(), properties->end(), [](const auto& property) {
  //       std::cout << "property name: " << property.first << ", property type name: " << property.second->name
  //                 << std::endl;
  //     });
  //   }

  //   auto methods = type_pair.second->methods;
  //   if (methods) {
  //     std::for_each(methods->begin(), methods->end(), [](const auto& method) {
  //       std::cout << "method name: " << method.first << ", method return type name: " << method.second.first->name
  //                 << ", method parameter type name: ";
  //       std::for_each(method.second.second.begin(), method.second.second.end(),
  //                     [](const auto& parameter) { std::cout << parameter->name << ", "; });
  //       std::cout << std::endl;
  //     });
  //   }
  // });

  // auto object = reflection::create_object("FirstObject");
  // std::cout << "object type name: " << object->get_type()->name << std::endl;
  // auto int_object = new reflection::IntObject;
  // int_object->value_ = 100;
  // object->set_property("age", reflection::ObjectPtr(int_object));
  return 0;
}
