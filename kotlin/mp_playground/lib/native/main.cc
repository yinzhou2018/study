#include <iostream>

#ifdef WIN32
#include <knative_api.h>
#else
#include <libknative_api.h>
#endif  // WIN32

extern "C" int increment_two(int x) {
  std::cout << "Incrementing " << x << " by 2" << std::endl;
  return x + 2;
}

int main() {
  std::cout << "Hello World!" << std::endl;
#ifdef WIN32
  auto lib = knative_symbols();
#else
  auto lib = libknative_symbols();
#endif  // WIN32

  auto str = lib->kotlin.root.com.example.strings("China");
  std::cout << str << std::endl;
  lib->DisposeString(str);

  auto field = lib->kotlin.root.com.example.Object.get_field(lib->kotlin.root.com.example.Object._instance());
  std::cout << "Object field value: " << field << std::endl;
  lib->DisposeString(field);
  lib->DisposeStablePointer(lib->kotlin.root.com.example.Object._instance().pinned);

  auto clazz = lib->kotlin.root.com.example.Clazz.Clazz();
  auto member = lib->kotlin.root.com.example.Clazz.member(clazz, 1);
  std::cout << "Clazz member value: " << lib->getNonNullValueOfULong(member) << std::endl;
  lib->DisposeStablePointer(member.pinned);
  lib->DisposeStablePointer(clazz.pinned);

  auto fn = lib->kotlin.root.com.example.supplyFun();
  auto result = lib->kotlin.root.com.example.acceptFun(fn);
  std::cout << "acceptFun result: " << result << std::endl;
  lib->DisposeString(result);
  lib->DisposeStablePointer(fn.pinned);

  auto r1 = lib->kotlin.root.com.example.c_acceptFun((void*)increment_two);
  std::cout << "c_acceptFun result: " << r1 << std::endl;

  auto f = lib->kotlin.root.com.example.c_supplyFun();
  auto r2 = ((int (*)(int))f)(1);
  std::cout << "c_supplyFun result: " << r2 << std::endl;
  auto r3 = lib->kotlin.root.com.example.c_acceptFun(f);
  std::cout << "c_acceptFun result: " << r3 << std::endl;

  return 0;
}