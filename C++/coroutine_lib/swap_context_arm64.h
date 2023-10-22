#pragma once

// 需要保存恢复的寄存器：x19 ~ x30, 共12个

// 保存上下文
#define SAVE_CONTEXT(sp)                       \
  asm volatile("sub %0, sp, 0x60" : "=r"(sp)); \
  asm volatile("stp x19, x20, [sp, -0x60]");   \
  asm volatile("stp x21, x22, [sp, -0x50]");   \
  asm volatile("stp x23, x24, [sp, -0x40]");   \
  asm volatile("stp x25, x26, [sp, -0x30]");   \
  asm volatile("stp x27, x28, [sp, -0x20]");   \
  asm volatile("stp x29, x30, [sp, -0x10]");

// 恢复上下文
#define RESTORE_CONTEXT(sp)                 \
  if (sp) {                                 \
    asm volatile("mov sp, %0" ::"r"(sp));   \
  }                                         \
  asm volatile("ldp x19, x20, [sp]");       \
  asm volatile("ldp x21, x22, [sp, 0x10]"); \
  asm volatile("ldp x23, x24, [sp, 0x20]"); \
  asm volatile("ldp x25, x26, [sp, 0x30]"); \
  asm volatile("ldp x27, x28, [sp, 0x40]"); \
  asm volatile("ldp x29, x30, [sp, 0x50]"); \
  asm volatile("mov sp, x29");              \
  asm volatile("ldp x29, x30, [sp]");       \
  asm volatile("add sp, sp, 0x10");         \
  asm volatile("br x30");

// 交换上下文
#define SWAP_CONTEXT(saved_sp, restored_sp) \
  SAVE_CONTEXT(saved_sp)                    \
  RESTORE_CONTEXT(restored_sp)