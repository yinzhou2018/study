#pragma once

typedef void (*PF_Routine_Entry)();
typedef void* CoroutineHandle;

CoroutineHandle coroutine_create(PF_Routine_Entry entry);
void coroutine_resume(CoroutineHandle handle);
void coroutine_yeild();
