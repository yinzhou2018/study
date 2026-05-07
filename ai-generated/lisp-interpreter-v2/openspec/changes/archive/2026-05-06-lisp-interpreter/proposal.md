## Why

需要一个基于 flex+bison+C++20 的极简 Lisp 解释器，用于学习编译原理和语言实现技术。选择 flex+bison 可以深入理解词法分析和语法生成的经典工具链，C++20 提供现代语言特性支持。同时支持 REPL 和文件执行两种模式，便于交互式开发与脚本执行。

## What Changes

- 新建基于 flex 的词法分析器，支持 Lisp 的原子类型（符号、数字、字符串、布尔值）及括号等语法元素
- 新建基于 bison 的语法分析器，构建 Lisp S-表达式 AST
- 新建 C++20 解释器核心，实现基本求值逻辑（算术运算、变量绑定、函数定义与调用、条件表达式）
- 新建 REPL 交互模式，支持 readline 式输入与实时求值
- 新建文件执行模式，从源文件读取并批量求值
- 新建构建系统（CMake），集成 flex/bison 代码生成

## Capabilities

### New Capabilities
- `lexer`: flex 词法分析器，将 Lisp 源码文本转换为 token 流
- `parser`: bison 语法分析器，将 token 流构建为 AST（S-表达式树）
- `evaluator`: 解释器求值引擎，支持基本 Lisp 语义（算术、变量、lambda、条件）
- `repl`: 交互式 REPL 模式，支持多行输入与实时求值输出
- `file-executor`: 文件执行模式，从 .lisp 文件读取源码并执行
- `build-system`: CMake 构建系统，集成 flex/bison 代码生成与 C++20 编译

### Modified Capabilities

（无，全新项目）

## Impact

- 依赖：flex、bison、C++20 兼容编译器（GCC 12+ / Clang 15+）
- 构建：CMake 3.20+，需集成 flex/bison 的代码生成步骤
- 输出：单一可执行文件，支持 `lisp`（REPL）和 `lisp <file>`（文件执行）两种调用方式
