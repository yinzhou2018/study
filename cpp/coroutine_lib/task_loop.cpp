#include "task_loop.h"
#include <mutex>
#include <queue>

static std::mutex task_queue_lock_;
static std::condition_variable task_queue_not_empty_;
static std::queue<std::function<void()>> task_queue_;
static bool need_quited_ = false;

void task_loop_run() {
  while (!need_quited_) {
    std::queue<std::function<void()>> temp_queue;
    {
      std::unique_lock lock(task_queue_lock_);
      if (task_queue_.empty())
        task_queue_not_empty_.wait(lock);
      task_queue_.swap(temp_queue);
    }

    while (!temp_queue.empty()) {
      auto task = temp_queue.front();
      temp_queue.pop();
      task();
    }
  }
}

void task_loop_quit() {
  auto task = []() { need_quited_ = true; };
  task_loop_post(task);
}

void task_loop_post(const std::function<void()>& task) {
  std::unique_lock lock(task_queue_lock_);
  task_queue_.push(task);
  task_queue_not_empty_.notify_one();
}