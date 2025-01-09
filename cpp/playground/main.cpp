#include <iostream>
// #include "advanced_feature_study.h"
// #include "coroutine_study.h"
// #include "exception_study.h"
// #include "multithread.h"
// #include "preload_file.h"
// #include "reflection_sample.h"
// #include "shared_ptr_study.h"
#include "async_await_study.h"

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
  async_await_study();
  // multithread_study();

  std::cout << "Hello, World!" << std::endl;
  return 0;
}
