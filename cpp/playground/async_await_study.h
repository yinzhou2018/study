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
#include <thread>

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
  if (coroutine_info.last_run_thread == std::this_thread::get_id()) {
    std::cout << "resume coroutine directly." << std::endl;
    coroutine_info.handle.resume();
  } else {
    std::cout << "resume coroutine with posting task." << std::endl;
    std::lock_guard guard(run_loops_lock_);
    auto iter = run_loops_.find(coroutine_info.last_run_thread);
    if (iter != run_loops_.end()) {
      std::function<void(void)> task = [=]() { coroutine_info.handle.resume(); };
      iter->second->post_task_ignore_result(task);
    }
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

template <typename T>
struct InnerValue {
  T value;
};  // InnerValue
template <>
struct InnerValue<void> {};

template <typename T>
struct CoroutineReturnTypeImpl {
  void set_value(const InnerValue<T>& value) {
    this->value = value;
    ready = true;
    if (wait_coroutine.handle) {
      CoroutineScheduler::get()->resume(wait_coroutine);
    }
  }

  bool ready{false};
  InnerValue<T> value;
  CoroutineInfo wait_coroutine;

  ~CoroutineReturnTypeImpl() { std::cout << "CoroutineReturnTypeImpl destructor..." << std::endl; }
};  // CoroutineReturnTypeImpl

template <typename T>
using CoroutineReturnTypeImplPtr = std::shared_ptr<CoroutineReturnTypeImpl<T>>;

template <typename T>
struct CoroutineReturnType {
  bool await_ready() { return impl_->ready; }
  void await_suspend(std::coroutine_handle<> h) {
    impl_->wait_coroutine = CoroutineInfo{h, std::this_thread::get_id()};
  }
  T await_resume() {
    if constexpr (!std::is_void_v<T>) {
      return impl_->value.value;
    }
  }

  CoroutineReturnTypeImplPtr<T> impl_;
};  // CoroutineReturnType

template <class T, class... ArgTypes>
struct std::coroutine_traits<CoroutineReturnType<T>, ArgTypes...> {
  struct promise_type {
    promise_type() {
      std::cout << "promise_type constuctor..." << std::endl;
      return_value_ = std::make_shared<CoroutineReturnTypeImpl<T>>();
    }
    ~promise_type() { std::cout << "promise_type destructor..." << std::endl; }
    CoroutineReturnType<T> get_return_object() { return CoroutineReturnType{return_value_}; }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_value(const T& value) { return_value_->set_value(InnerValue<T>{value}); }
    void unhandled_exception() { std::abort(); }

    CoroutineReturnTypeImplPtr<T> return_value_;
  };
};

template <class... ArgTypes>
struct std::coroutine_traits<CoroutineReturnType<void>, ArgTypes...> {
  struct promise_type {
    promise_type() {
      std::cout << "promise_type with void return type constructor..." << std::endl;
      return_value_ = std::make_shared<CoroutineReturnTypeImpl<void>>();
    }
    ~promise_type() { std::cout << "promise_type destructor..." << std::endl; }
    CoroutineReturnType<void> get_return_object() { return CoroutineReturnType{return_value_}; }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() { return_value_->set_value(InnerValue<void>{}); }
    void unhandled_exception() { std::abort(); }

    CoroutineReturnTypeImplPtr<void> return_value_;
  };
};

template <typename T>
struct AsyncAwaiter {
  AsyncAwaiter() { std::cout << "AsyncAwaiter constuctor..." << std::endl; }
  ~AsyncAwaiter() { std::cout << "AsyncAwaiter destructor..." << std::endl; }

  bool await_ready() { return ready; }

  void await_suspend(std::coroutine_handle<> handle) {
    wait_coroutine = CoroutineInfo{handle, std::this_thread::get_id()};
  }

  T await_resume() {
    if constexpr (!std::is_void_v<T>) {
      return value.value;
    }
  }

  void set_value(const InnerValue<T>& value) {
    this->value = value;
    ready = true;
    if (wait_coroutine.handle) {
      CoroutineScheduler::get()->resume(wait_coroutine);
    }
  }

  CoroutineInfo wait_coroutine;
  bool ready = false;
  InnerValue<T> value;
};  // AsyncAwaiter

template <typename T, typename... Args>
struct BackgroundComputeAwaiter : public AsyncAwaiter<T> {
  BackgroundComputeAwaiter(const std::function<T(Args...)>& func, Args&&... args) {
    auto task = [=, this]() {
      if constexpr (std::is_void_v<T>) {
        func(args...);
        this->set_value(InnerValue<void>{});
      } else {
        this->set_value(InnerValue<T>{func(args...)});
      }
    };
    std::thread{std::move(task)}.detach();
  }
};  // BackgroundComputeAwaiter

int async_calc(int a, int b) {
  for (auto _ : std::ranges::iota_view{0, 2}) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "async_calc finished" << std::endl;
  return a + b;
}

CoroutineReturnType<int> coroutine_one() {
  auto result = co_await BackgroundComputeAwaiter<int, int, int>{async_calc, 20, 30};
  std::cout << "coroutine_one result: " << result << std::endl;
  co_return result + 100;
}

CoroutineReturnType<void> coroutine_two() {
  auto result = co_await coroutine_one();
  std::cout << "coroutine_two result: " << result << std::endl;
}

CoroutineReturnType<void> coroutine_main(RunLoop* run_loop) {
  co_await coroutine_two();
  std::cout << "coroutine_main finished" << std::endl;
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