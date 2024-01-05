#include <stdio.h>
int g_val = 10;

int sub(int a, int b) {
  return a - b;
}

// __attribute__((visibility("default")))
void hello() {
  auto val = g_val;
  val += 1;
  printf("result: %d, %d\n", sub(20, 10), val);
}