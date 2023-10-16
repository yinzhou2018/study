#include "stack_coroutine.h"

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

enum CoroutineState { SUSPENDED = 0, RUNNING, DEAD };

constexpr auto COROUTINE_STACK_SIZE = 4096;
struct Coroutine {
  int8_t* stack;
  PF_Routine_Entry pf_entry;
  size_t* recover_sp;
  size_t* return_sp;
  CoroutineState state;
};

thread_local Coroutine* current_running_coroutine = nullptr;

void coroutine_entry(Coroutine* coroutine) {
  coroutine->pf_entry();
  current_running_coroutine = nullptr;
  auto return_sp = coroutine->return_sp;

  // TODO: 协程栈资源没有释放
  // munmap(coroutine->stack, COROUTINE_STACK_SIZE);
  // delete coroutine;

  RESTORE_CONTEXT(return_sp, 1);
}

CoroutineHandle coroutine_create(PF_Routine_Entry entry) {
  // 先简单点，不允许嵌套创建协程
  if (nullptr != current_running_coroutine)
    return nullptr;

  auto handle = new Coroutine;
  memset(handle, 0, sizeof(Coroutine));
  handle->pf_entry = entry;
  handle->stack = (int8_t*)stack_allocate(COROUTINE_STACK_SIZE);
  handle->recover_sp = (size_t*)(handle->stack + COROUTINE_STACK_SIZE - sizeof(size_t) * 2);  // 栈底

#ifdef __aarch64__
  // 31个通用寄存器 + 1个程序计数器
  *(handle->recover_sp - 1) = (size_t)coroutine_entry;
  handle->recover_sp -= 32;
  *handle->recover_sp = (int64_t)handle;  // 寄存器x0传递参数
#endif                                    // __aarch64__

  return (CoroutineHandle)handle;
}

void coroutine_resume(CoroutineHandle handle) {
  // 先简单点，不允许运行的协程调用
  if (nullptr == handle || nullptr != current_running_coroutine)
    return;

  Coroutine* real_handle = (Coroutine*)handle;
  if (SUSPENDED == real_handle->state) {
    real_handle->state = RUNNING;
    current_running_coroutine = real_handle;
    SWAP_CONTEXT(real_handle->return_sp, real_handle->recover_sp, 2);
  }
}

void coroutine_yeild() {
  if (nullptr == current_running_coroutine)
    return;
  current_running_coroutine->state = SUSPENDED;
  auto tmp_coroutine = current_running_coroutine;
  current_running_coroutine = nullptr;
  SWAP_CONTEXT(tmp_coroutine->recover_sp, tmp_coroutine->return_sp, 3);
}