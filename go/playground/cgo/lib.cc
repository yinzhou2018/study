#include "lib.h"

#include <stdio.h>
#include <iostream>

Point makePoint(int x, int y) {
  Point p = {x, y};
  return p;
}

void printMessage(const char* message) {
  printf("C function says: %s\n", message);
  std::cout << "C++ function says: " << message << std::endl;
}

int add(int a, int b) {
  return a + b;
}