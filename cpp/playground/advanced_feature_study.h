#pragma once
#include <string.h>

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
