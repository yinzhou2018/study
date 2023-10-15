#pragma once

// 保存上下文环境到自己的栈里，顺序为:（自栈顶向栈底，共32寄存器，大小：32*8 = 256字节 = 0x100）：
// x0, x1, ..., x30, pc
// TODO: 污染了x27寄存器
#define SAVE_CONTEXT(sp, tag)                                     \
  asm volatile("sub %0, sp, #0x100" : "=r"(sp));                  \
  asm volatile("sub x27, sp, #0x100");                            \
  asm volatile("stp x0, x1, [x27]");                              \
  asm volatile("stp x2, x3, [x27, #0x10]");                       \
  asm volatile("stp x4, x5, [x27, 0x20]");                        \
  asm volatile("stp x6, x7, [x27, 0x30]");                        \
  asm volatile("stp x8, x9, [x27, 0x40]");                        \
  asm volatile("stp x10, x11, [x27, 0x50]");                      \
  asm volatile("stp x12, x13, [x27, 0x60]");                      \
  asm volatile("stp x14, x15, [x27, 0x70]");                      \
  asm volatile("stp x16, x17, [x27, 0x80]");                      \
  asm volatile("stp x18, x19, [x27, 0x90]");                      \
  asm volatile("stp x20, x21, [x27, 0xa0]");                      \
  asm volatile("stp x22, x23, [x27, 0xb0]");                      \
  asm volatile("stp x24, x25, [x27, 0xc0]");                      \
  asm volatile("stp x26, x27, [x27, 0xd0]");                      \
  asm volatile("stp x28, x29, [x27, 0xe0]");                      \
  asm volatile("adrp x28, __return_label__" #tag "@PAGE");        \
  asm volatile("add x28, x28, __return_label__" #tag "@PAGEOFF"); \
  asm volatile("stp x30, x28, [x27, 0xf0]");

// 从自己的栈里恢复上下文，顺序为:（自栈顶向栈底，共32寄存器，大小：32*8 = 256字节 = 0x100）：
// x0, x1, ..., x30, pc
// TODO: 污染了x27寄存器
#define RESTORE_CONTEXT(sp, tag)                         \
  asm volatile("mov sp, %0" ::"r"(sp));                  \
  asm volatile("ldp x0, x1, [sp]");                      \
  asm volatile("ldp x2, x3, [sp, #0x10]");               \
  asm volatile("ldp x4, x5, [sp, 0x20]");                \
  asm volatile("ldp x6, x7, [sp, 0x30]");                \
  asm volatile("ldp x8, x9, [sp, 0x40]");                \
  asm volatile("ldp x10, x11, [sp, 0x50]");              \
  asm volatile("ldp x12, x13, [sp, 0x60]");              \
  asm volatile("ldp x14, x15, [sp, 0x70]");              \
  asm volatile("ldp x16, x17, [sp, 0x80]");              \
  asm volatile("ldp x18, x19, [sp, 0x90]");              \
  asm volatile("ldp x20, x21, [sp, 0xa0]");              \
  asm volatile("ldp x22, x23, [sp, 0xb0]");              \
  asm volatile("ldp x24, x25, [sp, 0xc0]");              \
  asm volatile("ldp x26, x27, [sp, 0xd0]");              \
  asm volatile("ldp x28, x29, [sp, 0xe0]");              \
  asm volatile("ldp x30, x27, [sp, 0xf0]");              \
  asm volatile("add sp, sp, #0x100");                    \
  asm volatile("br x27");                                \
  asm volatile("__return_label__" #tag ": mov sp, x29"); \
  asm volatile("ldp x29, x30, [sp]");                    \
  asm volatile("add sp, sp, #0x10");                     \
  asm volatile("ret");

#define SWAP_CONTEXT(saved_sp, restored_sp, tag) \
  SAVE_CONTEXT(saved_sp, tag)                    \
  RESTORE_CONTEXT(restored_sp, tag)