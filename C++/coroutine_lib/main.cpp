#include "stack_coroutine.h"

#include <iostream>

void coroutine1() {
  std::cout << "coroutine1" << std::endl;
  coroutine_yeild();
  std::cout << "coroutine2" << std::endl;
}

int main(int, char**) {
  std::cout << "main" << std::endl;
  auto handle = coroutine_create(coroutine1);
  coroutine_resume(handle);
  std::cout << "main1" << std::endl;
  coroutine_resume(handle);
  std::cout << "main2" << std::endl;
}
