#pragma once

#include <stddef.h>

void* stack_allocate(size_t size);
void stack_deallocate(void* p, size_t size);