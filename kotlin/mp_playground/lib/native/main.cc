#include <iostream>

#include <knative_api.h>

int main() {
  std::cout << "Hello World!" << std::endl;
  auto lib = knative_symbols();

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

  return 0;
}