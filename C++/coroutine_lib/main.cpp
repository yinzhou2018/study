#include <sys/mman.h>
#include <iostream>

struct SContext {
  std::string name;
};

int world() {
  std::cout << "world" << std::endl;
  return 10;
}

void hello() {
  auto stack = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  auto stack_bottom = (int8_t*)stack + (4096 / 16 - 1) * 16;

  std::cout << "hello 1" << std::endl;

  // asm volatile("mov x8, sp");
  // asm volatile("mov sp, %0" ::"r"(stack_bottom) : "x8");
  // asm volatile("stp x7, x8, [sp, #-16]!");
  asm volatile("bl %0" ::"i"(world) : "memory", "x0");
  // asm volatile("ldp x7, x8, [sp], #16");
  // asm volatile("mov sp, x8");
  // world();

  std::cout << "hello 2" << std::endl;
  munmap(stack, 4096);
}

int main(int, char**) {
  // auto handle = coroutine_create(routine1);
  // coroutine_resume(handle);
  // std::cout << "main 1" << std::endl;
  // coroutine_resume(handle);
  // std::cout << "main 2" << std::endl;
  hello();
  std::cout << "main" << std::endl;
  std::cout << "main2" << std::endl;
}
