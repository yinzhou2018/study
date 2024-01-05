#include <iostream>

// int sub(int a, int b);
void hello();
void hello2();

extern int add(int a, int b);

int main(int, char**) {
  std::cout << "Hello, from vis_test: " << add(10, 20) << std::endl;
  // std::cout << sub(20, 10) << std::endl;
  hello();
  hello2();
}
