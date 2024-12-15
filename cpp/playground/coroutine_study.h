#pragma once
#include <coroutine>
#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>

template <typename T>
struct coro_ret {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  //! 协程句柄
  handle_type coro_handle_;
  coro_ret(handle_type h) : coro_handle_(h) {}
  coro_ret(const coro_ret&) = delete;
  coro_ret(coro_ret&& s) : coro_handle_(s.coro_) { s.coro_handle_ = nullptr; }
  ~coro_ret() {
    //! 自行销毁
    if (coro_handle_)
      coro_handle_.destroy();
  }
  coro_ret& operator=(const coro_ret&) = delete;
  coro_ret& operator=(coro_ret&& s) {
    coro_handle_ = s.coro_handle_;
    s.coro_handle_ = nullptr;
    return *this;
  }

  //! 恢复协程，返回是否结束
  bool move_next() {
    coro_handle_.resume();
    return coro_handle_.done();
  }
  //! 通过promise获取数据，返回值
  T get() { return coro_handle_.promise().return_data_; }

  bool await_ready() const {
    std::cout << "await ready..." << std::endl;
    return coro_handle_.done();
  }

  void await_suspend(std::coroutine_handle<> h) const {
    std::cout << "await suspend..." << std::endl;
    std::thread([this, h] {
      while (!this->coro_handle_.done())
        std::this_thread::yield();
      h.resume();
    }).detach();
  }

  T await_resume() const {
    std::cout << "await resume..." << std::endl;
    return coro_handle_.promise().return_data_;
  }

  //! promise_type就是承诺对象，承诺对象用于协程内外交流
  struct promise_type {
    promise_type() = default;
    ~promise_type() = default;

    //! 生成协程返回值
    auto get_return_object() { return coro_ret<T>{handle_type::from_promise(*this)}; }

    //! 注意这个函数,返回的就是awaiter
    //! 如果返回std::suspend_never{}，就不挂起，
    //! 返回std::suspend_always{} 挂起
    //! 当然你也可以返回其他awaiter
    auto initial_suspend() {
      return std::suspend_never{};
      // return std::suspend_always{};
    }
    //! co_return 后这个函数会被调用
    void return_value(T v) {
      return_data_ = v;
      return;
    }
    //!
    auto yield_value(T v) {
      std::cout << "yield_value invoked." << std::endl;
      return_data_ = v;
      return std::suspend_always{};
    }
    //! 在协程最后退出后调用的接口。
    //! 若 final_suspend 返回 std::suspend_always 则需要用户自行调用
    //! handle.destroy() 进行销毁，但注意final_suspend被调用时协程已经结束
    //! 返回std::suspend_always并不会挂起协程（实测 VSC++ 2022）
    auto final_suspend() noexcept {
      std::cout << "final_suspend invoked." << std::endl;
      return std::suspend_always{};
    }
    //
    void unhandled_exception() { std::exit(1); }
    // 返回值
    T return_data_;
  };
};

template <typename T>
auto operator co_await(std::future<T> future) noexcept
  requires(!std::is_reference_v<T>)
{
  struct awaiter : std::future<T> {
    bool await_ready() const noexcept {
      using namespace std::chrono_literals;
      return this->wait_for(0s) != std::future_status::timeout;
    }

    void await_suspend(std::coroutine_handle<> cont) const {
      std::thread([this, cont] {
        this->wait();
        cont();
      }).detach();
    }

    T await_resume() { return this->get(); }
  };

  return awaiter{std::move(future)};
}

std::future<int> coroutine_3() {
  std::cout << "coroutine 3" << std::endl;
  auto fut = std::async(std::launch::async, []() -> int {
    auto val = 0;
    for (auto i = 0; i < 10; ++i) {
      val += i;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return val;
  });
  return fut;
}

// coro_ret<int> coroutine_2() {
//   std::cout << "coroutine 2" << std::endl;
//   auto val = co_await coroutine_3();
//   co_return (val + 10);
// }

coro_ret<int> coroutine_1() {
  std::cout << "coroutine 1" << std::endl;
  auto val = co_await coroutine_3();
  co_return (val + 10);
}


// 这就是一个协程函数
coro_ret<int> coroutine_root() {
  std::cout << "1. Thread id: " << std::this_thread::get_id() << std::endl;
  auto val = co_await coroutine_1();
  std::cout << "2. Thread id: " << std::this_thread::get_id() << std::endl;
  std::cout << "value is: " << val << std::endl;
  co_return 808;
}

void coroutine_study() {
  bool done = false;
  std::cout << "Start coroutine_root\n";
  auto c_r = coroutine_root();
  while (!done) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    done = c_r.coro_handle_.done();
  }
  std::cout << "Coroutine " << (done ? "is done " : "isn't done ") << "ret =" << c_r.get() << std::endl;
}