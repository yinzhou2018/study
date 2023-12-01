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

std::future<size_t> calc_total_len(const std::string& str1, const std::string& str2) {
  std::packaged_task<size_t()> task([&str1, &str2]() -> size_t { return str1.size() + str2.size(); });
  // task();
  return task.get_future();
}

int main(int, char**) {
  auto result = coroutine_await(calc_total_len, std::string("hello"), std::string("world"));
  std::cout << "result: " << result << std::endl;
  // std::cout << "main" << std::endl;
  // auto handle = coroutine_create(coroutine1);
  // auto i = 0;
  // while (coroutine_state(handle) != CoroutineState::DEAD) {
  //   coroutine_resume(handle);
  //   std::cout << "main loop count: " << i++ << std::endl;
  // }
  // std::cout << "main finished" << std::endl;
}
