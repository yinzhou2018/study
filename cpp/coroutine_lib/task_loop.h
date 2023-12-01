#pragma once
#include <functional>

void task_loop_run();
void task_loop_quit();
void task_loop_post(const std::function<void()>& task);
