#pragma once
#include <iostream>
#include <ranges>
#include <span>
#include <thread>
#include <variant>
#include <vector>

decltype(auto) add(auto a, auto b) {
  auto val = a + b;
  auto& val_ref = val;
  return val_ref;
}

struct Person {
  std::string name;
  int age;
};

template <typename T>
struct Protector {
  static constexpr bool value = false;
};

template <typename T>
int hello() {
  static_assert(Protector<T>::value, "hello");
}

struct Circle {};
struct Cube {};
struct Rectangle {};

using Shape = std::variant<Circle, Cube, Rectangle>;

void printShapeInfo(const Circle& circle) {
  std::cout << "Circle" << std::endl;
}

void printShapeInfo(const Cube& cube) {
  std::cout << "cube" << std::endl;
}

void printShapeInfo(const Rectangle& circle) {
  std::cout << "rectangle" << std::endl;
}

void visit(const Shape& shape) {
  auto idx = shape.index();
  if (0 == idx) {
    decltype(auto) val = std::get<Circle>(shape);
    printShapeInfo(val);
  }
}

template <typename T>
bool numEq(T left, T right) {
  constexpr bool is_integral = std::is_integral_v<T>;
  constexpr bool is_float = std::is_floating_point_v<T>;
  static_assert(is_integral || is_float, "Not supported type");
  if constexpr (is_integral) {
    std::cout << "integral compare." << std::endl;
    return left == right;
  } else if constexpr (is_float) {
    std::cout << "float compare." << std::endl;
    return left == right;
  }
}

constexpr int sum(int n) {
  auto result = 0;
  for (auto i : std::views::iota(1) | std::views::take(n)) {
    result += i;
  }
  return result;
}

void foo(int& i) {
  std::cout << "foo" << i << std::endl;
}

void foo(int&& i) {
  std::cout << "rvalue reference foo" << i << std::endl;
}

void bar(int& i) {
  std::cout << "bar" << i << std::endl;
}

void bar(int&& i) {
  std::cout << "rvalue reference bar" << i << std::endl;
}

auto return_ref(int& i) {
  return i;
}

decltype(auto) return_ref_2(int& i) {
  return i;
}

template <typename T>
auto add_with_type(T a, T b) {
  if constexpr (std::is_integral_v<T>) {
    return a + b;
  } else {
    return a - b;
  }
}

void advanced_feature_study() {
  int k = 10;
  auto k1 = return_ref(k);
  auto k2 = return_ref_2(k);
  decltype(auto) k3 = return_ref(k);
  decltype(auto) k4 = return_ref_2(k);
  auto&& i = 5;
  i = 10;
  int j = 10;
  std::cout << typeid(j).name() << std::endl;
  foo(i);
  foo(5);
  foo(std::forward<int>(i));

  auto l = add_with_type(1, 2);
  auto m = add_with_type(1.0, 2.0);
  std::cout << l << std::endl;
  std::cout << m << std::endl;
}