#include <iostream>

#include "algos_c.h"
#include "algos_cxx.h"

int main() {
  std::cout << "Hello, World!" << std::endl;
  std::cout << "C add: " << c_add(1, 2) << std::endl;
  std::cout << "C++ add: " << cxx_add(1, 2) << std::endl;
  return 0;
}