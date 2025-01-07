// Copyright (c) 2020, Tencent Inc.
// All rights reserved.

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <new>
#include <numeric>
#include <thread>
#include <utility>
#include <vector>

#define USE_LOCK 0

namespace trpc {

/**
 * @brief 线程安全的无锁队列的实现，支持多生产者多消费者(多个线程取/放数据)
 * 参考了mpmc_bounded_queue无锁队列(下见链接)的实现
 * http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
 * 论文: https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf 处见细节
 */
template <typename T>
class LockFreeQueue {
 public:
  enum {
    RT_OK = 0,     // 加入队列成功
    RT_FULL = 1,   // 队列已满
    RT_EMPTY = 2,  // 空队列
    RT_ERR = -1,   // 出错
  };

  /**
   * @brief 无锁队列的构造器
   * @param size 队列长度, 必须是2的幂, 以便使用&位运算的操作
   * @throw std::invalid_argument 如果队列长度不为2的幂
   */
  LockFreeQueue() : init_(false) {}

  ~LockFreeQueue() {}

  int Init(size_t size) {
    if (init_) {
      return RT_OK;
    }

    bool size_is_power_of_2 = (size >= 2) && ((size & (size - 1)) == 0);
    if (!size_is_power_of_2) {
      uint64_t tmp = 1;
      while (tmp <= size) {
        tmp <<= 1;
      }

      size = tmp;
    }

    mask_ = size - 1;
    capacity_ = size;
    std::cout << "LockFreeQueue capacity_= " << capacity_ << std::endl;

    elements_ = std::make_unique<Element[]>(capacity_);
    for (size_t i = 0; i < size; ++i) {
      elements_[i].sequence = i;
    }

    count_.store(0, std::memory_order_relaxed);

    enqueue_pos_.store(0, std::memory_order_relaxed);
    dequeue_pos_.store(0, std::memory_order_relaxed);

    return RT_OK;
  }

  /**
   * @brief 将data数据加入队列中
   * @param val 加入的数据
   * @return RT_OK/RT_FULL
   */
  int Enqueue(const T& val) {
#if USE_LOCK
    std::lock_guard<std::mutex> lock(lock_);
    if (count_.load(std::memory_order_relaxed) == capacity_) {
      return RT_FULL;
    }
    auto elem = &elements_[enqueue_pos_.load(std::memory_order_relaxed) & mask_];
    elem->data = val;
    count_.fetch_add(1, std::memory_order_relaxed);
    enqueue_pos_.fetch_add(1, std::memory_order_relaxed);
    return RT_OK;
#else
    Element* elem = nullptr;
    size_t pos = enqueue_pos_.load(std::memory_order_relaxed);

    while (true) {
      elem = &elements_[pos & mask_];
      size_t seq = elem->sequence.load(std::memory_order_acquire);
      intptr_t dif = (intptr_t)seq - (intptr_t)pos;

      // dif == 0: 当前位置为空
      // dif < 0: 表示位置有数据，seq是上一轮的下一个位置，而pos是新一轮的位置，会更大
      // dif > 0: 表示有个并行入队先行成功
      if (dif == 0) {
        if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
          break;
        }
      } else if (dif < 0) {
        if (pos - dequeue_pos_.load(std::memory_order_relaxed) == capacity_)
          return RT_FULL;  // queue full

        pos = enqueue_pos_.load(std::memory_order_relaxed);
      } else {
        pos = enqueue_pos_.load(std::memory_order_relaxed);
      }
    }

    elem->data = val;
    elem->sequence.store(pos + 1, std::memory_order_release);

    count_.fetch_add(1, std::memory_order_release);

    return RT_OK;
#endif  // USE_LOCK
  }

  /**
   * @brief 从队列取出数据到data中
   * @param val 取出的数据
   * @return RT_OK/RT_EMPTY
   */
  int Dequeue(T& val) {
#if USE_LOCK
    std::lock_guard<std::mutex> lock(lock_);
    if (count_.load(std::memory_order_relaxed) == 0) {
      enqueue_pos_.store(0, std::memory_order_relaxed);
      dequeue_pos_.store(0, std::memory_order_relaxed);
      return RT_EMPTY;
    }

    auto elem = &elements_[dequeue_pos_.load(std::memory_order_relaxed) & mask_];
    val = elem->data;
    count_.fetch_sub(1, std::memory_order_relaxed);
    dequeue_pos_.fetch_add(1, std::memory_order_relaxed);
    return RT_OK;
#else
    Element* elem = nullptr;
    size_t pos = dequeue_pos_.load(std::memory_order_relaxed);

    while (true) {
      elem = &elements_[pos & mask_];
      size_t seq = elem->sequence.load(std::memory_order_acquire);
      intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);

      // dif == 0: 当前位置有数据
      // dif < 0: 表示位置没有数据，此时seq==pos
      // dif > 0: 表示有一个并发出队先行成功
      if (dif == 0) {
        if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
          break;
        }
      } else if (dif < 0) {
        if (pos - enqueue_pos_.load(std::memory_order_relaxed) == 0)
          return RT_EMPTY;  // queue empty

        pos = dequeue_pos_.load(std::memory_order_relaxed);
      } else {
        pos = dequeue_pos_.load(std::memory_order_relaxed);
      }
    }

    val = elem->data;
    elem->sequence.store(pos + mask_ + 1, std::memory_order_release);

    count_.fetch_sub(1, std::memory_order_release);

    return RT_OK;
#endif  // USE_LOCK
  }

  uint32_t Size() const { return count_.load(std::memory_order_relaxed); }

  uint32_t Capacity() const { return static_cast<uint32_t>(capacity_); }

 private:
  LockFreeQueue(const LockFreeQueue& rhs) = delete;
  LockFreeQueue(LockFreeQueue&& rhs) = delete;

  LockFreeQueue& operator=(const LockFreeQueue& rhs) = delete;
  LockFreeQueue& operator=(LockFreeQueue&& rhs) = delete;

 private:
  static constexpr std::size_t hardware_destructive_interference_size = 64;

  // sequence用来判断当前位置是否为空，取值说明：
  // 1. 当前位置非空时：取值为之前插入值时enqueue_pos_的值加1
  // 2. 当前位置为空时：取值为下一次要在这里插入值时的enqueue_pos_值
  struct alignas(hardware_destructive_interference_size) Element {
    std::atomic<size_t> sequence;
    T data;
  };

  bool init_;

  std::size_t mask_;

  // 需要初始化
  std::size_t capacity_ = 0;

  std::unique_ptr<Element[]> elements_;

  std::atomic<int> count_;

  // enqueue_pos_与dequeue_pos_会持续增长，通过与capacity_取模运算来获取队列操作位置
  // enqueue_pos_ == dequeue_pos_ => 空队列
  // enqueue_pos_ - dequeue_pos_ == capacity_ => 队列满

  alignas(hardware_destructive_interference_size) std::atomic<size_t> enqueue_pos_;

  alignas(hardware_destructive_interference_size) std::atomic<size_t> dequeue_pos_;

  std::mutex lock_;
};

}  // namespace trpc

void lockfree_queue_test(int index) {
  constexpr int write_total_count = 1000 * 10000;
  constexpr int queue_size = 4;
  constexpr int write_thread_count = 4;
  constexpr int read_thread_count = 4;
  constexpr int write_count_per_thread = 1000 * 10000 / write_thread_count;

  auto read_flags = std::make_unique<int[]>(write_total_count);
  for (auto i = 0; i < write_total_count; ++i) {
    read_flags[i] = 0;
  }

  std::atomic<int> has_read_count{0};
  std::atomic<size_t> has_read_sum{0};
  trpc::LockFreeQueue<int> lf_queue;
  lf_queue.Init(queue_size);
  std::vector<std::thread> write_threads;
  std::vector<std::thread> read_threads;

  auto start = std::chrono::system_clock::now();

  for (int i = 0; i < write_thread_count; ++i) {
    write_threads.emplace_back(
        [&](int start_index) {
          int index = start_index;
          int end = start_index + write_count_per_thread;
          do {
            if (trpc::LockFreeQueue<int>::RT_OK == lf_queue.Enqueue(index)) {
              index++;
            }
          } while (index < end);
        },
        i * write_count_per_thread);
  }

  for (int i = 0; i < read_thread_count; ++i) {
    read_threads.emplace_back([&]() {
      int index = 0;
      do {
        int val = -1;
        if (trpc::LockFreeQueue<int>::RT_OK == lf_queue.Dequeue(val)) {
          has_read_sum.fetch_add(val, std::memory_order_relaxed);
          read_flags[val] = 1;
          has_read_count.fetch_add(1, std::memory_order_relaxed);
        }
      } while (has_read_count.load(std::memory_order_relaxed) < write_total_count);
    });
  }

  for (auto& t : write_threads) {
    t.join();
  }
  for (auto& t : read_threads) {
    t.join();
  }

  auto end = std::chrono::system_clock::now();
  auto result = 0;
  for (int i = 0; i < write_total_count; ++i) {
    result += read_flags[i];
  }
  std::cout << index << ". [" << ((result == write_total_count) ? "OK" : "FAILED") << ", result: " << result
            << ", total written count: " << write_total_count << ", write threads: " << write_thread_count
            << ", read threads: " << read_thread_count << "]lockfree_queue_test spends: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
}
