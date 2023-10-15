#include "stack_allocator_posix.h"

#ifdef __APPLE__
#include <sys/mman.h>
#endif  //__APPLE__

void* stack_allocate(size_t size) {
  return mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void stack_deallocate(void* p, size_t size) {
  munmap(p, size);
}