#pragma once
#include <cassert>
#include <condition_variable>
#include <coroutine>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <ranges>
struct CoroutineInfo {
  std::coroutine_handle<> handle;
  std::thread::id last_run_thread;
};  // CoroutineInfo

struct RunLoop;
struct CoroutineScheduler {
  static CoroutineScheduler* get();
  void resume(const CoroutineInfo& coroutine_info);
  void register_run_loop(RunLoop* run_loop);
  void unregister_run_loop(RunLoop* run_loop);

 private:
  using RunLoopMap = std::map<std::thread::id, RunLoop*>;
  RunLoopMap run_loops_;
  std::mutex run_loops_lock_;
};  // CoroutineScheduler

struct RunLoop {
  template <class R, class... ArgTypes>
  std::future<R> post_task(std::packaged_task<R(ArgTypes...)>&& task, ArgTypes&&... args);
  template <class R, class... ArgTypes>
  void post_task_ignore_result(const std::function<R(ArgTypes...)>& task, ArgTypes&&... args);

  void run();
  void quit();

 private:
  using Task = std::function<void(void)>;
  using TaskQueue = std::queue<Task>;

  void push_task(const Task& task);

  TaskQueue task_queue_;
  std::mutex task_lock_;
  std::condition_variable task_cond_variable_;
  bool quit_{false};
};  // RunLoop

CoroutineScheduler* CoroutineScheduler::get() {
  static CoroutineScheduler scheduler;
  return &scheduler;
}

void CoroutineScheduler::resume(const CoroutineInfo& coroutine_info) {
  std::lock_guard guard(run_loops_lock_);
  auto iter = run_loops_.find(coroutine_info.last_run_thread);
  if (iter != run_loops_.end()) {
    std::function<void(void)> task = [=]() { coroutine_info.handle.resume(); };
    iter->second->post_task_ignore_result(task);
  }
}

void CoroutineScheduler::register_run_loop(RunLoop* run_loop) {
  std::lock_guard guard(run_loops_lock_);
  run_loops_.insert({std::this_thread::get_id(), run_loop});
}

void CoroutineScheduler::unregister_run_loop(RunLoop* run_loop) {
  std::lock_guard guard(run_loops_lock_);
  auto iter = run_loops_.find(std::this_thread::get_id());
  if (iter != run_loops_.end()) {
    assert(iter->second == run_loop);
    run_loops_.erase(iter);
  }
}

template <class R, class... ArgTypes>
std::future<R> RunLoop::post_task(std::packaged_task<R(ArgTypes...)>&& task, ArgTypes&&... args) {
  auto fut = task.get_future();
  auto task_pointer = std::make_shared<std::packaged_task<R(ArgTypes...)>>(std::move(task));
  auto fn = [=]() { (*task_pointer)(args...); };
  push_task(fn);
  return fut;
}

template <class R, class... ArgTypes>
void RunLoop::post_task_ignore_result(const std::function<R(ArgTypes...)>& task, ArgTypes&&... args) {
  auto fn = [=]() { task(args...); };
  push_task(fn);
}

void RunLoop::run() {
  while (true) {
    TaskQueue queue;
    {
      auto lk = std::unique_lock(task_lock_);
      if (task_queue_.empty()) {
        task_cond_variable_.wait(lk);
      }
      queue = std::move(task_queue_);
    }

    while (!queue.empty()) {
      auto task = queue.front();
      queue.pop();
      task();
      if (quit_) {
        return;
      }
    }
  }
}

void RunLoop::quit() {
  Task task = [this]() { quit_ = true; };
  post_task_ignore_result(task);
}

void RunLoop::push_task(const Task& task) {
  std::lock_guard guard(task_lock_);
  task_queue_.push(task);
  task_cond_variable_.notify_one();
}

template <class T>
struct Awaiter {
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    auto info = CoroutineInfo{h, std::this_thread::get_id()};
    future_void_ = std::async(
                       std::launch::async,
                       [this](const auto& info) {
                         future_.wait();
                         CoroutineScheduler::get()->resume(info);
                       },
                       info)
                       .share();
  }
  T await_resume() {
    if constexpr (std::is_void_v<T>) {
      future_.get();
    } else {
      return future_.get();
    }
  }

  std::shared_future<T> future_;
  std::shared_future<void> future_void_;
};  // Awaiter

template <class T>
Awaiter<T> operator co_await(std::future<T>&& future) {
  return Awaiter{future.share()};
}

template <class T, class... ArgTypes>
struct std::coroutine_traits<std::future<T>, ArgTypes...> {
  struct promise_type {
    ~promise_type() { std::cout << "~promise_type" << std::endl; }
    std::future<T> get_return_object() { return promise_.get_future(); }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_value(const T& value) { promise_.set_value(value); }
    void unhandled_exception() { std::abort(); }

    std::promise<T> promise_;
  };
};

template <class... ArgTypes>
struct std::coroutine_traits<std::future<void>, ArgTypes...> {
  struct promise_type {
    ~promise_type() { std::cout << "~promise_type" << std::endl; }
    std::future<void> get_return_object() { return promise_.get_future(); }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() { promise_.set_value(); }
    void unhandled_exception() { std::abort(); }

    std::promise<void> promise_;
  };
};

std::future<int> async_calc() {
  return std::async(std::launch::async, []() {
    for (auto _ : std::ranges::iota_view{0, 5}) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "async_calc finished" << std::endl;
    return 100;
  });
}

std::future<int> coroutine_one() {
  auto result = co_await async_calc();
  std::cout << "coroutine_one result: " << result << std::endl;
  co_return result + 100;
}

std::future<void> coroutine_two() {
  auto result = co_await async_calc();
  std::cout << "coroutine_two result: " << result << std::endl;
}

std::future<void> coroutine_main(RunLoop* run_loop) {
  auto fut_one = coroutine_one();
  auto fut_two = coroutine_two();
  std::cout << "get future" << std::endl;
  auto result = co_await std::move(fut_one);
  std::cout << "result: " << result << std::endl;
  co_await std::move(fut_two);
  run_loop->quit();
}

void async_await_main(RunLoop* run_loop) {
  coroutine_main(run_loop);
}

void async_await_study() {
  auto run_loop = RunLoop{};
  CoroutineScheduler::get()->register_run_loop(&run_loop);
  std::function<void(RunLoop*)> task(async_await_main);
  run_loop.post_task_ignore_result(task, &run_loop);
  run_loop.run();
  CoroutineScheduler::get()->unregister_run_loop(&run_loop);
  std::cout << "run loop finished" << std::endl;
}