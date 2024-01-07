#pragma once
#include <string.h>
#include <variant>

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

void advanced_feature_study() {
  // std::visit()
}