#pragma once
#include "reflection.h"

#include "reflection_object.inl"

// 可以添加其他成员变量或函数...
private:
  int add_age_impl(int base, int increment);

END_REFLECTION_TYPE()

void test_reflection_sample();