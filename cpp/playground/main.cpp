
#include "windows/screen_capture.h"

#include <iostream>

int main(int, char**) {
  std::cout << "Hello, Welcome to cpp playground!" << std::endl;

#ifdef _WIN32
  win_screen_capture_test();
#endif  // _WIN32

  // ranges_study();
  // async_await_study();
  return 0;
}
