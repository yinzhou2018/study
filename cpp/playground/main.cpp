#include <iostream>
#include "advanced_feature_study.h"
#include "coroutine_study.h"
#include "exception_study.h"
#include "preload_file.h"

int main(int, char**) {
  std::cout << "Hello, Welcome to playground...\n";
  // advanced_feature_study();
  preload_file(R"(E:\virtual_human_code\ue\virtual_human_ue51\Saved\1.0.1\Windows\VirtualHumanUE51\Content\Paks\base.pak)");
  return 0;
}
