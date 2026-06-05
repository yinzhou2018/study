## Context

本项目为全新C++命令行计算器应用，从零构建。项目须遵循Chromium C++编码规范，头文件使用`.h`后缀，实现文件使用`.cc`后缀，充分利用C++20特性。项目需要TDD驱动开发，所有模块需先编写测试再实现功能。

## Goals / Non-Goals

**Goals:**
- 实现支持四则运算、括号嵌套、一元负号的数学表达式求值
- 构建清晰的分层架构：词法分析 → 语法分析 → 求值 → 交互界面
- 建立完整的TDD开发流程，确保高测试覆盖率
- 提供可复用的CMake构建系统，支持编译、测试、格式化

**Non-Goals:**
- 不支持科学计算函数（sin、cos、log等）
- 不支持变量赋值和存储
- 不支持历史记录持久化
- 不提供GUI界面

## Decisions

### D1: 使用递归下降解析器

**选择**: 递归下降（Recursive Descent）解析器

**替代方案**:
- 运算符优先级解析器（Pratt Parser）：更灵活但实现复杂度更高
- YACC/Bison生成器：引入外部依赖，不符合轻量原则

**理由**: 四则运算文法简单且无歧义，递归下降解析器直观易读，便于TDD逐步构建。每个文法规则对应一个解析函数，符合单一职责原则。

### D2: AST节点类型使用C++20 std::variant和std::unique_ptr

**选择**: `std::variant` 表达不同AST节点类型，`std::unique_ptr` 管理递归结构

**替代方案**:
- 传统类继承+虚函数：运行时多态，堆分配开销
- 标记联合体（Tagged Union）：手动管理类型标签，容易出错

**理由**: `std::variant` 提供类型安全的访问模式（std::visit），编译期确定类型，避免虚函数开销。配合`std::unique_ptr`实现树形结构的自动内存管理，无需手动delete。

### D3: 错误处理使用std::expected（C++23）或自定义Result类型

**选择**: 自定义`Result<T>`模板，内部使用`std::variant<T, Error>`

**替代方案**:
- 异常：Chromium规范限制异常使用
- std::optional：无法携带错误信息
- C++23 std::expected：编译器支持尚不充分

**理由**: 遵循Chromium编码规范避免异常，自定义Result类型可携带错误详情（位置、原因），比std::optional更有诊断价值。

### D4: 目录结构

**选择**:
```
calculator-v2/
├── CMakeLists.txt
├── .clang-format
├── src/
│   ├── main.cc
│   ├── lexer.h / lexer.cc
│   ├── parser.h / parser.cc
│   ├── evaluator.h / evaluator.cc
│   └── repl.h / repl.cc
├── include/
│   └── calculator/
│       ├── token.h
│       ├── ast.h
│       └── result.h
└── test/
    ├── lexer_test.cc
    ├── parser_test.cc
    ├── evaluator_test.cc
    └── repl_test.cc
```

**替代方案**:
- 单一src/目录无分层：文件增多后难以管理
- 完全header-only：增加编译时间，降低封装性

**理由**: 公共接口放在include/calculator/下供测试和外部使用，实现细节放在src/中。测试独立目录便于CTest发现和运行。

### D5: Google Test作为测试框架

**选择**: Google Test + Google Mock

**替代方案**:
- Catch2：更轻量但Chromium项目更常用GTest
- doctest：编译更快但生态较小

**理由**: Google Test是C++生态最主流的测试框架，与CMake/CTest集成成熟，文档丰富。

## Risks / Trade-offs

- [浮点精度问题] → 使用double进行计算，对溢出和精度丢失场景输出警告；不追求任意精度
- [表达式复杂度无上限] → 不设递归深度限制，极端嵌套可能导致栈溢出；后续版本可增加深度检查
- [Unicode输入] → 仅支持ASCII字符，非ASCII输入直接报词法错误
