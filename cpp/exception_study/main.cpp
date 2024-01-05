#include <iostream>

void visit_nullptr() {
  std::cout << __FUNCTION__ << std::endl;
  int* p = nullptr;
  std::cout << *p << std::endl;
}

void visit_invalid_address() {
  std::cout << __FUNCTION__ << std::endl;
  int* p = (int*)0x1234;
  std::cout << *p << std::endl;
}

void visit_outofrange() {
  std::cout << __FUNCTION__ << std::endl;
  int* p = new int[5]{0, 1, 2, 3, 4};
  std::cout << *p << std::endl;
  std::cout << *(p + 5) << std::endl;
  delete[] p;
}

void double_free() {
  std::cout << __FUNCTION__ << std::endl;
  int* p = new int{10};
  std::cout << *p << std::endl;
  delete p;
  delete p;
}

void visit_dangling_ptr() {
  std::cout << __FUNCTION__ << std::endl;
  int* p = new int{10};
  std::cout << *p << std::endl;
  delete p;
  std::cout << *p << std::endl;
}

void visit_misaligned_address() {
  std::cout << __FUNCTION__ << std::endl;
  int* p = new int{10};
  std::cout << "size: " << sizeof(int) << "," << *p << std::endl;
  int* misaligned_p = (int*)((char*)p + 1);
  std::cout << (long)misaligned_p << ": " << *misaligned_p << std::endl;
}

int main(int argc, char* argv[]) {
  try {
    // visit_nullptr();
    visit_misaligned_address();
  } catch (...) {
    std::cout << "crashed..." << std::endl;
  }
  return 0;
}