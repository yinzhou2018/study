#include <atomic>
#include <iostream>

#include "lockfree_queue.h"

int main(int, char**) {
  std::cout << "atomic<bool>::is_always_lock_free: " << std::atomic<bool>::is_always_lock_free << std::endl;
  std::cout << "atomic<int>::is_always_lock_free: " << std::atomic<int>::is_always_lock_free << std::endl;
  std::cout << "atomic<int64_t>::is_always_lock_free: " << std::atomic<int64_t>::is_always_lock_free << std::endl;
  std::cout << "Hello world, from console-demo!" << std::endl;
  int i = 0;
  while (true) {
    lockfree_queue_test(++i);
  }
}
