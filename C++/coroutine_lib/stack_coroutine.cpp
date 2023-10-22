#include "stack_coroutine.h"
#include <vector>

#ifdef __aarch64__
#include "swap_context_arm64.h"
#else
#include "swap_context_x64.h"
#endif  //__aarch64__

#ifdef _WIN32
#include "stack_allocator_win.h"
#else
#include "stack_allocator_posix.h"
#endif  // _WIN32

#include <stdint.h>
#include <string.h>

struct Coroutine {
  int8_t* stack;
  size_t stack_size;
  PF_Routine_Entry pf_entry;
  size_t* recover_sp;
  size_t* return_sp;
  Coroutine* return_coroutine;
  CoroutineState state;
};

thread_local Coroutine* g_current_running_coroutine = nullptr;
thread_local std::vector<Coroutine*> g_coroutines;

static void coroutine_main() {
  g_current_running_coroutine->pf_entry();

  asm volatile("mov sp, %0"::"r"(g_current_running_coroutine->return_sp));
  auto tmp_coroutine = g_current_running_coroutine;
  g_current_running_coroutine = nullptr;
  coroutine_destroy(tmp_coroutine);
  
  RESTORE_CONTEXT(nullptr);
}

CoroutineState coroutine_state(CoroutineHandle handle) {
  auto iter = std::find(g_coroutines.begin(), g_coroutines.end(), (Coroutine*)handle);
  return (iter != g_coroutines.end()) ? (*iter)->state : CoroutineState::DEAD;
}

CoroutineHandle coroutine_create(PF_Routine_Entry entry, size_t initial_stack_size) {
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
  handle->recover_sp = (size_t*)(handle->stack + aligned_initial_stack_size - sizeof(size_t) * 2);  // 栈底

#ifdef __aarch64__
  // 模拟保存上下文
  *(handle->recover_sp - 1) = (size_t)coroutine_main;
  *(handle->recover_sp - 2) = (size_t)(handle->recover_sp);
  *(handle->recover_sp - 4) = (size_t)(handle->recover_sp - 2);
  handle->recover_sp -= 14;
#endif  // __aarch64__

  g_coroutines.push_back(handle);
  return (CoroutineHandle)handle;
}

bool coroutine_destroy(CoroutineHandle handle) {
  // 运行过程中不能自毁
  if (g_current_running_coroutine == handle)
    return false;

  auto iter = std::find(g_coroutines.begin(), g_coroutines.end(), handle);
  if (iter == g_coroutines.end())
    return false;

  auto real_handle = *iter;
  g_coroutines.erase(iter);
  stack_deallocate(real_handle->stack, real_handle->stack_size);
  delete real_handle;
  return true;
}

void coroutine_resume(CoroutineHandle handle) {
  // 已经在运行中
  if (handle == g_current_running_coroutine)
    return;

  auto iter = std::find(g_coroutines.begin(), g_coroutines.end(), (Coroutine*)handle);
  if (iter == g_coroutines.end())
    return;

  auto real_handle = (Coroutine*)handle;
  real_handle->state = CoroutineState::RUNNING;
  if (g_current_running_coroutine)
    g_current_running_coroutine->state = CoroutineState::SUSPENDED;
  real_handle->return_coroutine = g_current_running_coroutine;
  g_current_running_coroutine = real_handle;

  SWAP_CONTEXT(real_handle->return_sp, real_handle->recover_sp);
}

void coroutine_yeild() {
  if (nullptr == g_current_running_coroutine)
    return;

  g_current_running_coroutine->state = CoroutineState::SUSPENDED;
  auto tmp_coroutine = g_current_running_coroutine;
  g_current_running_coroutine = g_current_running_coroutine->return_coroutine;
  if (g_current_running_coroutine)
    g_current_running_coroutine->state = CoroutineState::RUNNING;

  SWAP_CONTEXT(tmp_coroutine->recover_sp, tmp_coroutine->return_sp);
}