#include "stack_coroutine.h"
#include <stdint.h>
#include <sys/mman.h>

// 上下文环境保存到各自的栈里，顺序为（自栈顶向下）：pc, x0, x1, ..., x31
#define SWAP_CONTEXT(saved_context, restored_context)

// arm64寄存器
struct Context {
  int64_t sp;
};

enum CoroutineState { SUSPENDED = 0, RUNNING, DEAD };

struct Coroutine {
  int8_t* stack;
  PF_Routine_Entry pf_entry;
  Context recover_context;
  Context return_context;
  CoroutineState state;
};

thread_local Coroutine* current_running_coroutine = nullptr;

// TODO:coroutine结束没有释放内存
void coroutine_entry(Coroutine* coroutine) {
  coroutine->pf_entry();
  coroutine->state = DEAD;
  current_running_coroutine = nullptr;
  SWAP_CONTEXT(coroutine->recover_context, coroutine->return_context);
}

CoroutineHandle coroutine_create(PF_Routine_Entry entry) {
  // 先简单点，不允许嵌套创建协程
  if (nullptr != current_running_coroutine)
    return nullptr;

  auto handle = new Coroutine;
  handle = {0};
  handle->pf_entry = entry;
  handle->stack = (int8_t*)mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  handle->recover_context.sp = (int64_t)(handle->stack + (4096 / 16 - 1) * 16);

  // 31个通用寄存器 + 1个程序计数器
  int64_t* sp = (int64_t*)handle->recover_context.sp;
  *sp = (int64_t)coroutine_entry;
  *(sp - 1) = (int64_t)handle;  // 寄存器x0传递参数
  handle->recover_context.sp -= 32 * 8;

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
    // SWAP_CONTEXT(real_handle->return_context, real_handle->recover_context);

    // 上下文环境保存到各自的栈里，顺序为:（自栈底向栈顶，共32寄存器，大小：32*8 = 256字节 = 0x100）：
    // x0, x1, ..., x30, pc
    asm volatile("sub sp, sp, #0x100");
    asm volatile("stp x0, x1, [sp]");
    asm volatile("stp x2, x3, [sp, #0x10]");
    asm volatile("stp x4, x5, [sp, 0x20]");
    asm volatile("stp x6, x7, [sp, 0x30]");
    asm volatile("stp x8, x9, [sp, 0x40]");
    asm volatile("stp x10, x11, [sp, 0x50]");
    asm volatile("stp x12, x13, [sp, 0x60]");
    asm volatile("stp x14, x15, [sp, 0x70]");
    asm volatile("stp x16, x17, [sp, 0x80]");
    asm volatile("stp x18, x19, [sp, 0x90]");
    asm volatile("stp x20, x21, [sp, 0xa0]");
    asm volatile("stp x22, x23, [sp, 0xb0]");
    asm volatile("stp x24, x25, [sp, 0xc0]");
    asm volatile("stp x26, x27, [sp, 0xd0]");
    asm volatile("stp x28, x29, [sp, 0xe0]");
    asm volatile("adrp x27, __return_label__@PAGE");
    asm volatile("add x27, x27, __return_label__@PAGEOFF");
    asm volatile("stp x30, x27, [sp, 0xf0]");
    asm volatile("mov %0, sp" : "=r"(real_handle->return_context.sp));

    // 从栈里恢复上下文环境，TODO: 污染了x27寄存器
    asm volatile("mov sp, %0" ::"r"(real_handle->recover_context.sp));
    asm volatile("ldp x0, x1, [sp]");
    asm volatile("ldp x2, x3, [sp, #0x10]");
    asm volatile("ldp x4, x5, [sp, 0x20]");
    asm volatile("ldp x6, x7, [sp, 0x30]");
    asm volatile("ldp x8, x9, [sp, 0x40]");
    asm volatile("ldp x10, x11, [sp, 0x50]");
    asm volatile("ldp x12, x13, [sp, 0x60]");
    asm volatile("ldp x14, x15, [sp, 0x70]");
    asm volatile("ldp x16, x17, [sp, 0x80]");
    asm volatile("ldp x18, x19, [sp, 0x90]");
    asm volatile("ldp x20, x21, [sp, 0xa0]");
    asm volatile("ldp x22, x23, [sp, 0xb0]");
    asm volatile("ldp x24, x25, [sp, 0xc0]");
    asm volatile("ldp x26, x27, [sp, 0xd0]");
    asm volatile("ldp x28, x29, [sp, 0xe0]");
    asm volatile("ldp x30, x27, [sp, 0xf0]");
    asm volatile("add sp, sp, #0x100");
    asm volatile("br x27");
    asm volatile("__return_label__: ret");
  }
}

void coroutine_yeild() {
  if (nullptr == current_running_coroutine)
    return;
  current_running_coroutine->state = SUSPENDED;
  auto tmp_coroutine = current_running_coroutine;
  current_running_coroutine = nullptr;
  SWAP_CONTEXT(tmp_coroutine->recover_context, tmp_coroutine->return_context);
}