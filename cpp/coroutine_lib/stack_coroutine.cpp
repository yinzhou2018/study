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

#ifdef _WIN32
// windows-x64平台longjmp实现及其复杂诡异，会导致stack coruption;
// 基于setjmp反汇编代码对应重新实现私有longjmp接口搞定问题。
#define longjmp longjmp_private
static void longjmp_private(jmp_buf, int) {
  asm volatile("mov %edx, %eax");
  asm volatile("mov (%rcx), %rdx");
  asm volatile("mov 8(%rcx), %rbx");
  asm volatile("mov 16(%rcx), %rsp");
  asm volatile("mov 24(%rcx), %rbp");
  asm volatile("mov 32(%rcx), %rsi");
  asm volatile("mov 40(%rcx), %rdi");
  asm volatile("mov 48(%rcx), %r12");
  asm volatile("mov 56(%rcx), %r13");
  asm volatile("mov 64(%rcx), %r14");
  asm volatile("mov 72(%rcx), %r15");
  asm volatile("ldmxcsr 88(%rcx)");
  asm volatile("fnclex");
  asm volatile("fldcw 92(%rcx)");
  asm volatile("movdqa 96(%rcx), %xmm6");
  asm volatile("movdqa 112(%rcx), %xmm7");
  asm volatile("movdqa 128(%rcx), %xmm8");
  asm volatile("movdqa 144(%rcx), %xmm9");
  asm volatile("movdqa 160(%rcx), %xmm10");
  asm volatile("movdqa 176(%rcx), %xmm11");
  asm volatile("movdqa 192(%rcx), %xmm12");
  asm volatile("movdqa 208(%rcx), %xmm13");
  asm volatile("movdqa 224(%rcx), %xmm14");
  asm volatile("movdqa 240(%rcx), %xmm15");
  asm volatile("jmp *80(%rcx)");
}
#endif  // _WIN32

struct Coroutine {
  int8_t* stack;
  int8_t* stack_bottom;
  size_t stack_size;
  PF_Coroutine_Entry pf_entry;
  jmp_buf self_context;
  jmp_buf caller_context;
  CoroutineState state;
};  // Coroutine

static thread_local Coroutine* current_running_coroutine_ = nullptr;
static thread_local std::vector<Coroutine*> coroutine_list_;

static void coroutine_main() {
  current_running_coroutine_->state = CoroutineState::RUNNING;
  current_running_coroutine_->pf_entry();
  current_running_coroutine_->state = CoroutineState::DEAD;
  longjmp(current_running_coroutine_->caller_context, 1);
}

CoroutineState coroutine_state(CoroutineHandle handle) {
  auto iter = std::find(coroutine_list_.begin(), coroutine_list_.end(), (Coroutine*)handle);
  return (iter != coroutine_list_.end()) ? (*iter)->state : CoroutineState::DEAD;
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
  coroutine_list_.push_back(handle);
  return (CoroutineHandle)handle;
}

CoroutineHandle coroutine_get_current() {
  return current_running_coroutine_;
}

bool coroutine_destroy(CoroutineHandle handle) {
  // 运行过程中不能自毁
  if (current_running_coroutine_ == handle)
    return false;

  auto coroutine = (Coroutine*)handle;
  auto iter = std::find(coroutine_list_.begin(), coroutine_list_.end(), coroutine);
  if (iter == coroutine_list_.end())
    return false;

  coroutine_list_.erase(iter);
  stack_deallocate(coroutine->stack, coroutine->stack_size);
  delete coroutine;
  return true;
}

void coroutine_resume(CoroutineHandle handle) {
  // 已经在运行中
  if (handle == current_running_coroutine_)
    return;

  auto iter = std::find(coroutine_list_.begin(), coroutine_list_.end(), (Coroutine*)handle);
  if (iter == coroutine_list_.end())
    return;

  auto current_coroutine = current_running_coroutine_;
  if (current_coroutine)
    current_coroutine->state = CoroutineState::SUSPENDED;

  auto next_coroutine = *iter;
  if (0 == setjmp(next_coroutine->caller_context)) {
    if (CoroutineState::CREATED == next_coroutine->state) {
      current_running_coroutine_ = next_coroutine;
#ifdef __aarch64__
      asm volatile("mov sp, %0" ::"r"(next_coroutine->stack_bottom));
#else
      asm volatile("mov %0, %%rsp" ::"r"(next_coroutine->stack_bottom));
#endif  //__aarch64__
      coroutine_main();
    } else {
      longjmp(next_coroutine->self_context, 1);
    }
  } else {
    if (CoroutineState::DEAD == current_running_coroutine_->state)
      coroutine_destroy(current_running_coroutine_);
    current_running_coroutine_ = current_coroutine;
    if (current_coroutine)
      current_coroutine->state = CoroutineState::RUNNING;
  }
}

void coroutine_yeild() {
  if (nullptr == current_running_coroutine_)
    return;

  auto current_coroutine = current_running_coroutine_;
  current_coroutine->state = CoroutineState::SUSPENDED;
  if (0 == setjmp(current_coroutine->self_context)) {
    longjmp(current_coroutine->caller_context, 1);
  } else {
    current_coroutine->state = CoroutineState::RUNNING;
    current_running_coroutine_ = current_coroutine;
  }
}