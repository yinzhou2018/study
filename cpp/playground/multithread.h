#pragma once

#include <atomic>
#include <cassert>
#include <iostream>
#include <ranges>
#include <thread>
#include <vector>

// std::atomic<bool> x{false}, y{false};
auto x = false, y = false;

void thread1() {
  // x.store(true, std::memory_order_relaxed);  // (1)
  x = true;
}

void thread2() {
  // y.store(true, std::memory_order_relaxed);  // (2)
  y = true;
}

std::atomic<int> z{0};

void read_x_then_y() {
  while (!x)
    ;  // (3)
  if (y)
    ++z;  // (4)
}

void read_y_then_x() {
  while (!y)
    ;  // (5)
  if (x)
    ++z;  // (6)
}

void multithread_study() {
  for (auto i : std::views::iota(0, 100000)) {
    std::thread a(thread1), b(thread2), c(read_x_then_y), d(read_y_then_x);
    a.join(), b.join(), c.join(), d.join();
    if (0 == z.load()) {
      std::cout << "got it." << std::endl;
    }
  }
}