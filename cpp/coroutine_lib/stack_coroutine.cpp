#include "stack_coroutine.h"

#ifdef _WIN32
#include "stack_allocator_win.h"
#else
#include "stack_allocator_posix.h"
#endif  // _WIN32

#include <setjmp.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <vector>

struct Coroutine {
  int8_t* stack;
  int8_t* stack_bottom;
  size_t stack_size;
  PF_Coroutine_Entry pf_entry;
  jmp_buf self_context;
  jmp_buf caller_context;
  CoroutineState state;
};  // Coroutine

thread_local Coroutine* g_current_running_coroutine = nullptr;
thread_local std::vector<Coroutine*> g_coroutines;

static void coroutine_main() {
  g_current_running_coroutine->state = CoroutineState::RUNNING;
  g_current_running_coroutine->pf_entry();
  g_current_running_coroutine->state = CoroutineState::DEAD;
  longjmp(g_current_running_coroutine->caller_context, 1);
}

CoroutineState coroutine_state(CoroutineHandle handle) {
  auto iter = std::find(g_coroutines.begin(), g_coroutines.end(), (Coroutine*)handle);
  return (iter != g_coroutines.end()) ? (*iter)->state : CoroutineState::DEAD;
}

CoroutineHandle coroutine_create(PF_Coroutine_Entry entry, size_t initial_stack_size) {
  if (initial_stack_size < MIN_COROUTINE_STACK_SIZE)
    return nullptr;

  // 大小对齐到2倍sizeof(size_t)大小，否则执行会有对齐异常
  auto remainder = initial_stack_size % (sizeof(size_t) * 2);
  auto aligned_initial_stack_size =
      (0 == remainder) ? initial_stack_size : (initial_stack_size + sizeof(size_t) * 2 - remainder);

  auto handle = new Coroutine;
  memset(handle, 0, sizeof(Coroutine));
  handle->stack_size = aligned_initial_stack_size;
  handle->pf_entry = entry;
  handle->stack = (int8_t*)stack_allocate(aligned_initial_stack_size);
  handle->stack_bottom = handle->stack + aligned_initial_stack_size - sizeof(size_t) * 2;
  g_coroutines.push_back(handle);
  return (CoroutineHandle)handle;
}

bool coroutine_destroy(CoroutineHandle handle) {
  // 运行过程中不能自毁
  if (g_current_running_coroutine == handle)
    return false;

  auto coroutine = (Coroutine*)handle;
  auto iter = std::find(g_coroutines.begin(), g_coroutines.end(), coroutine);
  if (iter == g_coroutines.end())
    return false;

  g_coroutines.erase(iter);
  stack_deallocate(coroutine->stack, coroutine->stack_size);
  delete coroutine;
  return true;
}

void coroutine_resume(CoroutineHandle handle) {
  // 已经在运行中
  if (handle == g_current_running_coroutine)
    return;

  auto iter = std::find(g_coroutines.begin(), g_coroutines.end(), (Coroutine*)handle);
  if (iter == g_coroutines.end())
    return;

  auto current_coroutine = g_current_running_coroutine;
  if (current_coroutine)
    current_coroutine->state = CoroutineState::SUSPENDED;

  auto next_coroutine = *iter;
  if (0 == setjmp(next_coroutine->caller_context)) {
    if (CoroutineState::CREATED == next_coroutine->state) {
      g_current_running_coroutine = next_coroutine;
#ifdef __aarch64__
      asm volatile("mov sp, %0" ::"r"(next_coroutine->stack_bottom));
#else   // TODO
#endif  //__aarch64__
      coroutine_main();
    } else {
      longjmp(next_coroutine->self_context, 1);
    }
  } else {
    if (CoroutineState::DEAD == g_current_running_coroutine->state)
      coroutine_destroy(g_current_running_coroutine);
    g_current_running_coroutine = current_coroutine;
    if (current_coroutine)
      current_coroutine->state = CoroutineState::RUNNING;
  }
}

void coroutine_yeild() {
  if (nullptr == g_current_running_coroutine)
    return;

  auto current_coroutine = g_current_running_coroutine;
  current_coroutine->state = CoroutineState::SUSPENDED;
  if (0 == setjmp(current_coroutine->self_context)) {
    longjmp(current_coroutine->caller_context, 1);
  } else {
    current_coroutine->state = CoroutineState::RUNNING;
    g_current_running_coroutine = current_coroutine;
  }
}