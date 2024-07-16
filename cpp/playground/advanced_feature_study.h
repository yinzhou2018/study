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


void advanced_feature_study() {
  constexpr auto val = sum(10);
  std::cout << val << std::endl;
}