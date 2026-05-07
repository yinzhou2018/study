## Context

从零构建一个极简 Lisp 解释器，作为学习编译原理的实践项目。项目采用经典的 flex（词法分析）+ bison（语法分析）+ C++20（求值引擎）三层架构。当前无任何已有代码，为全新实现。

目标语言为 Scheme 风格的 Lisp 方言，支持 S-表达式、基本数据类型、算术运算、变量绑定、lambda 函数、条件表达式及基本列表操作。

## Goals / Non-Goals

**Goals:**
- 实现完整的 lex→parse→eval 管线，从源码文本到求值结果
- 支持 REPL 交互模式（带提示符、多行输入、表达式续行）
- 支持文件执行模式（`lisp <file>`）
- 支持 Lisp 核心语义：符号、数字、字符串、布尔值、quote、lambda、define、if、cond、let、基本列表操作（car/cdr/cons/list）
- 遵循 C++20 编码标准（Chromium 风格，.h/.cc 后缀，类成员≤7，函数≤40行）
- 使用 CMake 构建系统，自动集成 flex/bison 代码生成

**Non-Goals:**
- 不实现宏系统（defmacro/syntax-rules）
- 不实现尾调用优化（TCO）
- 不实现垃圾回收优化（使用 shared_ptr 引用计数即可）
- 不支持 continuations / call/cc
- 不支持标准库丰富扩展（仅内置基本函数）
- 不实现模块/包系统

## Decisions

### 1. AST 表示：variant + 递归结构

**选择**：使用 `std::variant<Nil, Bool, Number, String, Symbol, Pair, Lambda>` 表示 Lisp 值，Pair 为链表节点（car/cdr），Lambda 闭包捕获环境。

**替代方案**：虚函数继承体系（基类 + 各子类）。放弃原因：堆分配开销大，类型判断需 dynamic_cast，不符合值语义风格。

**理由**：variant 提供 std::visit 模式匹配，与 Lisp 多态求值天然契合，且栈分配为主，性能更好。Pair 递归结构自然表示列表和嵌套 S-表达式。

### 2. 环境模型：链式作用域

**选择**：Environment 类持有 `std::unordered_map<std::string, Value>` 变量绑定 + `std::shared_ptr<Environment>` 指向外层作用域，形成链式作用域链。

**替代方案**：单一扁平哈希表 + 变量名前缀。放弃原因：不符合 Lisp 词法作用域语义，lambda 闭包实现复杂。

**理由**：链式作用域直接映射 Lisp 的词法作用域规则，lambda 闭包只需捕获定义时的环境指针即可，实现简洁且语义正确。

### 3. 词法/语法分析：flex + bison

**选择**：使用 flex 生成词法分析器，bison 生成 LALR(1) 语法分析器。

**替代方案**：手写递归下降解析器。放弃原因：本项目目的之一是学习 flex/bison 工具链。

**理由**：flex/bison 是编译原理经典工具，自动生成高效的分析代码。bison 的 LALR(1) 足以处理 Lisp 语法（语法本身极简），且支持位置追踪用于错误报告。

### 4. 构建系统：CMake + 外部依赖查找

**选择**：CMake 3.20+，使用 `find_package(FLEX)` 和 `find_package(BISON)` 集成代码生成步骤。

**替代方案**：Makefile 直接调用 flex/bison。放弃原因：可移植性差，与 IDE 集成不便。

**理由**：CMake 是 C++ 项目标准构建系统，`flex_add_module` / `bison_target` 命令原生支持代码生成步骤集成，跨平台兼容。

### 5. REPL 实现：GNU readline

**选择**：使用 GNU readline 库提供行编辑、历史记录和括号匹配提示。

**替代方案**：简单 std::cin/std::cout 循环。放弃原因：用户体验差，无法编辑已输入内容。

**理由**：readline 提供 Emacs 风格行编辑、上下箭头历史、可编程补全，是 REPL 应用的标准选择，且 macOS/Linux 均预装。

## Risks / Trade-offs

- **[bison 生成代码与 C++20 兼容性]** → bison 默认生成 C 代码，需配置 `%language "c++"` 和 `%header` 指令生成 C++ 解析器类，并验证与 C++20 编译选项无冲突
- **[shared_ptr 循环引用]** → Pair 链表和 Lambda 闭包可能产生循环引用导致内存泄漏 → 初期可接受，Non-Goal 已排除 GC 优化，后续可引入弱引用打破循环
- **[Lisp 语法歧义]** → Lisp 语法本身几乎无歧义（前缀表达式），bison 处理无压力，但需注意 quote 简写 `'expr` 的词法/语法处理
- **[错误报告质量]** → flex/bison 默认错误信息简陋 → 需利用 `%locations` 追踪行列号，自定义 `yy::parser::error` 提供友好错误信息
