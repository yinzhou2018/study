#pragma once
#include "task_loop.h"

#include <stddef.h>
#include <future>

typedef void (*PF_Coroutine_Entry)();
typedef void* CoroutineHandle;
enum class CoroutineState { CREATED = 0, SUSPENDED, RUNNING, DEAD };

// TODO: 没有实现栈尺寸的按需动态增长
constexpr auto MIN_COROUTINE_STACK_SIZE = 1 * 1024 * 1024;  // 1M

CoroutineHandle coroutine_create(PF_Coroutine_Entry entry, size_t initial_stack_size = MIN_COROUTINE_STACK_SIZE);
CoroutineHandle coroutine_get_current();
bool coroutine_destroy(CoroutineHandle handle);
void coroutine_resume(CoroutineHandle handle);
void coroutine_yeild();
CoroutineState coroutine_state(CoroutineHandle handle);

template <typename Callable, typename... Args>
auto coroutine_await(Callable&& func, Args&&... args) {
  auto fut = func(std::forward<Args>(args)...);
  auto shared_fut = fut.share();
  auto current = coroutine_get_current();
  (void)std::async(std::launch::async, [shared_fut, current]() {
    shared_fut.wait();
    auto task = [current]() { coroutine_resume(current); };
    task_loop_post(task);
  });
  coroutine_yeild();
  return shared_fut.get();
}