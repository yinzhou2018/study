#include <iostream>
#include "advanced_feature_study.h"
#include "coroutine_study.h"
#include "exception_study.h"
#include "multithread.h"
#include "preload_file.h"
#include "reflection_study.h"
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

int main(int, char**) {
  std::cout << "Hello, Welcome to playground...\n";
  // advanced_feature_study();
  // preload_file(R"(E:\virtual_human_code\ue\virtual_human_ue51\Saved\1.0.1\Windows\VirtualHumanUE51\Content\Paks\base.pak)");
  // multithread_study();
  // shared_ptr_study();
  // core_foundation_study();
  std::for_each(reflection::g_types.begin(), reflection::g_types.end(), [](const auto& type_pair) {
    std::cout << "type name: " << type_pair.first << ", parent type name : "
              << ((type_pair.second->parent != nullptr) ? type_pair.second->parent->name : "null") << std::endl;
    auto properties = type_pair.second->properties;
    if (properties) {
      std::for_each(properties->begin(), properties->end(), [](const auto& property) {
        std::cout << "property name: " << property.first << ", property type name: " << property.second->name
                  << std::endl;
      });
    }

    auto methods = type_pair.second->methods;
    if (methods) {
      std::for_each(methods->begin(), methods->end(), [](const auto& method) {
        std::cout << "method name: " << method.first << ", method return type name: " << method.second.first->name
                  << ", method parameter type name: ";
        std::for_each(method.second.second.begin(), method.second.second.end(),
                      [](const auto& parameter) { std::cout << parameter->name << ", "; });
        std::cout << std::endl;
      });
    }
  });

  auto object = reflection::create_object("FirstObject");
  std::cout << "object type name: " << object->get_type()->name << std::endl;
  auto int_object = new reflection::IntObject;
  int_object->value_ = 100;
  object->set_property("age", reflection::ObjectPtr(int_object));
  return 0;
}
