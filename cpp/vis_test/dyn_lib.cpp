#include <stdio.h>
int sub(int, int);
void hello();

void hello2() {
  printf("result: %d", sub(20, 10));
  hello();
}

int add(int a, int b) {
  return a + b + 10;
}