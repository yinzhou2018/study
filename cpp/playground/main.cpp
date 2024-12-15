#include <iostream>
#include "advanced_feature_study.h"
#include "coroutine_study.h"
#include "exception_study.h"
#include "multithread.h"
#include "preload_file.h"
#include "reflection_sample.h"
#include "shared_ptr_study.h"

#include "lib/utils.h"

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

struct DemoStruct {
  DemoStruct() { std::cout << "DemoStruct" << std::endl; }
  ~DemoStruct() { std::cout << "~DemoStruct" << std::endl; }
};

constexpr int mul(int a, int b) { return a * b; }

int main(int, char**) {
  advanced_feature_study();
  // preload_file(R"(E:\virtual_human_code\ue\virtual_human_ue51\Saved\1.0.1\Windows\VirtualHumanUE51\Content\Paks\base.pak)");
  // multithread_study();
  // shared_ptr_study();
  // core_foundation_study();
  // test_reflection_sample();
  std::vector<DemoStruct> demo_structs;
  std::cout << "size: " << demo_structs.size() << std::endl;
  std::cout << "result: " << add(1, 2) << std::endl;
  return 0;
}
