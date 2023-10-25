#pragma once

// 需要被调用者保存恢复的寄存器：x19 ~ x30, 共12个

// 需要构建出调用yield后类似的上下文环境，
// 才能统一在resume调用正常恢复，所以这里需要保存14个寄存器，分两部分：
// 1. 模拟yield调用入口保存的返回地址（携程入口地址）及上一个栈帧基址，共2个
// 2. 常规的被调者需要保存的寄存器：x19~x30，共12个
#define BUILD_INITIAL_CONTEXT(sp, entry) \
  *(sp - 1) = (size_t)entry;             \
  *(sp - 2) = (size_t)(sp);              \
  *(sp - 4) = (size_t)(sp - 2);          \
  sp -= 14;

#define SAVE_CONTEXT(sp)                       \
  asm volatile("sub %0, sp, 0x60" : "=r"(sp)); \
  asm volatile("stp x19, x20, [sp, -0x60]");   \
  asm volatile("stp x21, x22, [sp, -0x50]");   \
  asm volatile("stp x23, x24, [sp, -0x40]");   \
  asm volatile("stp x25, x26, [sp, -0x30]");   \
  asm volatile("stp x27, x28, [sp, -0x20]");   \
  asm volatile("stp x29, x30, [sp, -0x10]");

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

#define SWAP_CONTEXT(save_sp, restore_sp) \
  SAVE_CONTEXT(save_sp)                   \
  RESTORE_CONTEXT(restore_sp)