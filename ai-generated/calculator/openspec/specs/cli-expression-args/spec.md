# CLI Expression Args

## Purpose

定义通过命令行参数传入数学表达式并执行的能力，包括表达式解析执行、错误处理和退出码约定。

## Requirements

### Requirement: 命令行表达式参数执行
系统 SHALL 支持通过命令行参数传入数学表达式，解析执行后输出结果并退出。当 `main()` 接收到命令行参数时，将所有参数以空格拼接为表达式字符串，执行 Lexer→Parser→Evaluator 管线，将计算结果输出到 stdout 后以退出码 0 退出。

#### Scenario: 单参数表达式执行
- **WHEN** 用户运行 `calculator "1+2"`
- **THEN** 系统输出 "3" 到 stdout 并以退出码 0 退出

#### Scenario: 多参数拼接表达式执行
- **WHEN** 用户运行 `calculator 1 + 2`
- **THEN** 系统将参数拼接为 "1 + 2"，输出 "3" 到 stdout 并以退出码 0 退出

#### Scenario: 复杂表达式执行
- **WHEN** 用户运行 `calculator "2*pi"`
- **THEN** 系统输出 "6.28318..." 到 stdout 并以退出码 0 退出

### Requirement: 命令行参数模式错误处理
系统 SHALL 在命令行参数表达式求值失败时将错误信息输出到 stderr，并以退出码 1 退出。错误包括词法分析错误、语法分析错误和运行时求值错误。

#### Scenario: 词法错误
- **WHEN** 用户运行 `calculator "1&2"`
- **THEN** 系统将错误信息输出到 stderr 并以退出码 1 退出

#### Scenario: 语法错误
- **WHEN** 用户运行 `calculator "1++2"`
- **THEN** 系统将错误信息输出到 stderr 并以退出码 1 退出

#### Scenario: 除零错误
- **WHEN** 用户运行 `calculator "1/0"`
- **THEN** 系统将除零错误信息输出到 stderr 并以退出码 1 退出

### Requirement: 命令行参数模式无欢迎信息
系统 SHALL 在命令行参数模式下不输出 REPL 欢迎信息（如 "Calculator v1.0"）和退出提示。

#### Scenario: 参数模式无欢迎信息
- **WHEN** 用户运行 `calculator "1+2"`
- **THEN** 系统仅输出 "3"，不包含欢迎信息或提示

### Requirement: 无参数时保持 REPL 行为
系统 SHALL 在无命令行参数时保持现有 REPL 交互行为不变。

#### Scenario: 无参数启动 REPL
- **WHEN** 用户运行 `calculator`（无参数）
- **THEN** 系统进入 REPL 交互模式，显示欢迎信息，行为与当前一致
