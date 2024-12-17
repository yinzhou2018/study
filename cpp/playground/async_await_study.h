#pragma once
#include <cassert>
#include <condition_variable>
#include <coroutine>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <queue>

struct CoroutineInfo {
  std::coroutine_handle<> handle;
  std::thread::id run_thread_id;
};  // CoroutineInfo

struct Scheduler {
  virtual void resume(const CoroutineInfo& coroutine_info) = 0;
};

struct RunLoop {
  using Task = std::function<void(void)>;
  using TaskQueue = std::queue<Task>;

  template <class R, class... ArgTypes>
  std::future<R> post_task(std::packaged_task<R(ArgTypes...)>&& task, ArgTypes&&... args) {
    auto fut = task.get_future();
    auto task_pointer = std::make_shared<std::packaged_task<R(ArgTypes...)>>(std::move(task));
    auto fn = [=]() { (*task_pointer)(args...); };
    push_task(fn);
    return fut;
  }

  void run() {
    while (true) {
      TaskQueue queue_;
      {
        auto lk = std::unique_lock(task_lock_);
        if (task_queue_.empty()) {
          task_cond_variable_.wait(lk);
        }
        queue_ = std::move(task_queue_);
      }

      while (!queue_.empty()) {
        auto task = queue_.front();
        task_queue_.pop();
        task();
        if (quit_) {
          return;
        }
      }
    }
  }

  void quit() {
    std::packaged_task<void(void)> task([this]() { quit_ = true; });
    post_task(std::move(task));
  }

 private:
  void push_task(const Task& task) {
    std::lock_guard guard(task_lock_);
    task_queue_.push(task);
    task_cond_variable_.notify_one();
  }

  TaskQueue task_queue_;
  std::mutex task_lock_;
  std::condition_variable task_cond_variable_;
  bool quit_{false};
};  // RunLoop

struct DefaultScheduler : public Scheduler {
  void resume(const CoroutineInfo& coroutine_info) override {
    std::lock_guard guard(run_loops_lock_);
    auto iter = run_loops_.find(coroutine_info.run_thread_id);
    if (iter != run_loops_.end()) {
      std::packaged_task<void(void)> task([=]() { coroutine_info.handle.resume(); });
      iter->second->post_task(std::move(task));
    }
  }

  void register_run_loop(RunLoop* run_loop) {
    std::lock_guard guard(run_loops_lock_);
    run_loops_.insert({std::this_thread::get_id(), run_loop});
  }

  void unregister_run_loop(RunLoop* run_loop) {
    std::lock_guard guard(run_loops_lock_);
    auto iter = run_loops_.find(std::this_thread::get_id());
    if (iter != run_loops_.end()) {
      assert(iter->second == run_loop);
      run_loops_.erase(iter);
    }
  }

 private:
  using RunLoopMap = std::map<std::thread::id, RunLoop*>;
  RunLoopMap run_loops_;
  std::mutex run_loops_lock_;
};  // DefaultScheduler
static DefaultScheduler g_def_scheduler_;

template <class T>
struct Awaiter {
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    auto info = CoroutineInfo{h, std::this_thread::get_id()};
    auto th = std::thread(
        [this](auto info) {
          future_.wait();
          scheduler_->resume(info);
        },
        info);
    th.detach();
  }
  T await_resume() { return future_.get(); }

  std::shared_future<T> future_;
  Scheduler* scheduler_ = nullptr;
};

template <class T>
Awaiter<T> operator co_await(std::future<T> future) {
  return Awaiter{future.share(), &g_def_scheduler_};
}

void print_i(int& i) {
  std::cout << "lvalue: " << i << std::endl;
}

// void print_i(int i) {
//   std::cout << "no reference: " << i << std::endl;
// }

void print_i(int&& i) {
  std::cout << "rvalue: " << i << std::endl;
}

template <class R, class... ArgTypes>
void call_task(std::packaged_task<R(ArgTypes...)>&& task, ArgTypes&&... args) {}

void async_await_study() {
  print_i(10);
  std::packaged_task<int(const int&, const int&)> task([](const int a, const int b) { return a + b; });
}