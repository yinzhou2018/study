#include <iostream>
#include "advanced_feature_study.h"
#include "coroutine_study.h"
#include "exception_study.h"
#include "multithread.h"
#include "preload_file.h"
#include "shared_ptr_study.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
void core_foundation_study() {
  // 创建一个CFStringRef类型的字符串
  CFStringRef myString = CFStringCreateWithCString(NULL, "你好", kCFStringEncodingUTF8);

  char buffer[100] = {0};
  // 将CFStringRef转换为C字符串
  CFStringGetCString(myString, buffer, sizeof(buffer), kCFStringEncodingUTF8);

  // 打印字符串
  // if (cString) {
  std::cout << buffer << std::endl;
  // }

  // 释放CFStringRef
  CFRelease(myString);
}
#endif  // __APPLE__

int main(int, char**) {
  std::cout << "Hello, Welcome to playground...\n";
  // advanced_feature_study();
  // preload_file(R"(E:\virtual_human_code\ue\virtual_human_ue51\Saved\1.0.1\Windows\VirtualHumanUE51\Content\Paks\base.pak)");
  // multithread_study();
  // shared_ptr_study();
  core_foundation_study();
  return 0;
}
