#include "stack_coroutine.h"

#include <iostream>

void coroutine2() {
  for (auto i = 0; i < 10; ++i) {
    std::cout << "coroutine2 loop count: " << i << std::endl;
    coroutine_yeild();
  }
}

void coroutine1() {
  for (auto i = 0; i < 10; ++i) {
    std::cout << "coroutine1 loop count: " << i << std::endl;
    coroutine_yeild();
  }

  auto handle = coroutine_create(coroutine2);
  auto i = 0;
  while (coroutine_state(handle) != CoroutineState::DEAD) {
    coroutine_resume(handle);
    coroutine_yeild();
    coroutine_destroy(handle);
  }
}

int main(int, char**) {
  std::cout << "main" << std::endl;
  auto handle = coroutine_create(coroutine1);
  auto i = 0;
  while (coroutine_state(handle) != CoroutineState::DEAD) {
    coroutine_resume(handle);
    std::cout << "main loop count: " << i++ << std::endl;
  }
  std::cout << "main finished" << std::endl;
}
