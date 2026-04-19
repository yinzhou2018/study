#ifndef EVAL_HPP
#define EVAL_HPP

#include "ast.hpp"

// 求值函数
ValuePtr eval(ValuePtr expr, std::shared_ptr<Env> env);

// 创建全局环境，包含所有内置函数
std::shared_ptr<Env> createGlobalEnv();

#endif  // EVAL_HPP