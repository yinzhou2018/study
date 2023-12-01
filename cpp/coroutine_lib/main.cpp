#include "stack_coroutine.h"
#include "task_loop.h"

#include <iostream>

std::future<int> sum(int base) {
  auto fut = std::async(std::launch::async, [base]() -> int {
    auto value = base;
    for (auto i = 1; i <= 100; ++i)
      value += i;
    return value;
  });
  return fut;
}

std::future<int> dummy2() {
  std::cout << "dummy2 started..." << std::endl;
  auto result = coroutine_await(sum, 0);
  std::cout << "dummy2 result: " << result << std::endl;
  return sum(result);
}

std::future<int> dummy1() {
  std::cout << "dummy1 started..." << std::endl;
  auto result = coroutine_await(dummy2);
  std::cout << "dummy1 result: " << result << std::endl;
  return sum(result);
}

void coroutine1() {
  std::cout << "coroutine1 started..." << std::endl;
  auto result = coroutine_await(dummy1);
  std::cout << "coroutine1 result: " << result << std::endl;
  task_loop_quit();
}

int main(int, char**) {
  std::cout << "main started..." << std::endl;
  auto task = []() {
    auto handle = coroutine_create(coroutine1);
    coroutine_resume(handle);
    std::cout << "task finished..." << std::endl;
  };
  task_loop_post(task);
  task_loop_run();
  std::cout << "main finished..." << std::endl;
}
