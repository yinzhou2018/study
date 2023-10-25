#pragma once
#include <stddef.h>

typedef void (*PF_Coroutine_Entry)();
typedef void* CoroutineHandle;
enum class CoroutineState { SUSPENDED = 0, RUNNING, DEAD };
constexpr auto MIN_COROUTINE_STACK_SIZE = 2048;  // 2k

CoroutineHandle coroutine_create(PF_Coroutine_Entry entry, size_t initial_stack_size = MIN_COROUTINE_STACK_SIZE);
bool coroutine_destroy(CoroutineHandle handle);
void coroutine_resume(CoroutineHandle handle);
void coroutine_yeild();
CoroutineState coroutine_state(CoroutineHandle handle);
