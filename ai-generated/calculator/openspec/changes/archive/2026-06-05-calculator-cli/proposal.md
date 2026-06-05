## Why

需要开发一个基于C++和CMake构建的计算器命令行应用，提供基本的数学运算能力，同时作为C++项目工程化实践的起点，验证C++20特性、Clang-format代码规范、TDD开发流程的落地。

## What Changes

- 新增C++命令行计算器应用，支持基本四则运算（加减乘除）及括号表达式求值
- 新增CMake构建系统配置，支持跨平台编译
- 新增基于Google Test的单元测试框架集成
- 新增表达式解析器，支持运算符优先级和括号嵌套
- 新增REPL交互式输入模式，支持逐行输入表达式并输出结果

## Capabilities

### New Capabilities

- `expression-parser`: 表达式词法分析与语法解析，将数学表达式字符串转换为可求值的抽象语法树
- `calculator-engine`: 四则运算求值引擎，执行AST计算并返回结果
- `cli-interface`: 命令行交互界面，提供REPL模式读取用户输入并输出计算结果
- `build-system`: CMake构建系统配置，管理编译、测试、格式化等目标

### Modified Capabilities

（无已有能力需要修改）

## Impact

- 新增C++源码目录结构（src/, include/, test/）
- 新增CMakeLists.txt及关联配置文件
- 新增第三方依赖：Google Test（测试框架）
- 无现有代码或API受影响（全新项目）
