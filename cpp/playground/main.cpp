
#ifdef _WIN32
#include "windows/screen_capture.h"
#elif defined(__APPLE__)
#include "mac/screen_capture.h"
#else
#error "Unsupported platform"
#endif
#include <iostream>

int main(int, char**) {
  std::cout << "Hello, Welcome to cpp playground!" << std::endl;

  screen_capture_test();

  // ranges_study();
  // async_await_study();
  return 0;
}
