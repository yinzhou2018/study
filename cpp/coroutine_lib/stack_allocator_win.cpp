#include "stack_allocator_win.h"

#include <Windows.h>

void* stack_allocate(size_t size) {
  return ::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void stack_deallocate(void* p, size_t size) {
  ::VirtualFree(p, size, MEM_RELEASE);
}