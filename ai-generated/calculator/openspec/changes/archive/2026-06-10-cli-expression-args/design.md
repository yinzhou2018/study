## Context

当前计算器仅支持两种输入模式：REPL 交互模式（`isatty` 为真时使用 linenoise）和管道输入模式（从 stdin 逐行读取）。主入口 `main()` 直接构造 `Repl` 对象并调用 `Run()`，无命令行参数处理。`Repl::ProcessLine()` 已包含完整的 词法分析→语法分析→求值 管线，但其输出固定到 `out_` 流（stdout），错误信息也输出到同一流。

## Goals / Non-Goals

**Goals:**
- 支持 `calculator "1+2"` 形式直接传入表达式参数，执行后输出结果并退出
- 支持 `calculator 1 + 2` 多参数拼接为表达式，无需加引号
- 表达式求值错误输出到 stderr 并以非零退出码退出
- 无参数时保持现有 REPL 行为不变

**Non-Goals:**
- 不实现 `--help`、`--version` 等标准 CLI 选项（可后续迭代）
- 不支持从文件读取表达式
- 不改变 REPL 模式下的任何行为

## Decisions

### 1. 多参数用空格拼接为表达式

`calculator 1 + 2` 等同于 `calculator "1+2"`：将 `argv[1..]` 用空格连接后作为表达式输入。

**替代方案**: 仅支持单个参数——要求用户必须加引号。拒绝理由：不符合 Unix 命令行工具惯例，用户体验差。

### 2. 表达式求值逻辑复用 `Repl::ProcessLine()` 的管线

提取 `ProcessLine()` 中的 词法分析→语法分析→求值 逻辑为独立的自由函数 `EvaluateExpression()`，供 `main()` 直接调用，避免依赖 `Repl` 类。

**替代方案**: 在 `main()` 中构造 `Repl` 对象并注入单行输入。拒绝理由：`Repl` 职责是循环交互，单次执行不应承担欢迎信息、退出命令判断等职责。

### 3. 错误输出到 stderr，结果输出到 stdout

符合 Unix 管道惯例：`calculator "1+2"` 的 `3` 可安全管道至下游，而错误不会污染数据流。

### 4. 退出码约定

- 成功：退出码 0
- 表达式求值失败（词法/语法/运行时错误）：退出码 1

## Risks / Trade-offs

- **[多参数拼接歧义]** → `calculator sin pi` 拼接为 `sin pi`，词法分析会生成 `Identifier("sin") Identifier("pi")`，语法分析将报错。这是预期行为——用户应写 `calculator "sin(pi)"`。风险低，可通过文档说明缓解。
- **[shell 特殊字符]** → `*` 等字符在 shell 中会被展开，但这是 shell 层面的问题，不应在应用层处理。用户需加引号或转义。
