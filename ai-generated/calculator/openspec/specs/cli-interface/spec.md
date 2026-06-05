# CLI Interface

## Purpose

定义计算器命令行交互界面的能力，包括REPL模式、退出命令、错误提示、空行处理和启动欢迎信息。

## Requirements

### Requirement: REPL交互模式
系统 SHALL 提供REPL（Read-Eval-Print Loop）交互模式，逐行读取用户输入的数学表达式，计算并直接输出结果（不带前缀）。在终端交互模式下，系统 SHALL 在等待输入前显示 "> " 提示符；在管道输入模式下，系统 SHALL 不显示提示符。

#### Scenario: 交互式计算
- **WHEN** 用户在终端REPL中输入 "1+2" 并按回车
- **THEN** 系统输出 "3"（不带"= "前缀）并显示 "> " 提示符等待下一行输入

#### Scenario: 连续多次计算
- **WHEN** 用户依次在终端REPL中输入 "2+3" 和 "4*5"
- **THEN** 系统依次输出 "5" 和 "20"（均不带"= "前缀）

#### Scenario: 管道输入模式
- **WHEN** 通过管道输入表达式（如 `echo "1+2" | calculator`）
- **THEN** 系统输出 "3"（不带"= "前缀）且不显示 "> " 提示符

### Requirement: 退出命令
系统 SHALL 支持退出命令，用户输入"quit"或"exit"时退出REPL。

#### Scenario: 通过quit退出
- **WHEN** 用户输入 "quit"
- **THEN** 系统正常退出，返回退出码0

#### Scenario: 通过exit退出
- **WHEN** 用户输入 "exit"
- **THEN** 系统正常退出，返回退出码0

### Requirement: 错误提示
系统 SHALL 在用户输入无效表达式时输出友好错误提示，并继续REPL循环。

#### Scenario: 输入非法表达式
- **WHEN** 用户输入 "1++2"
- **THEN** 系统输出错误提示信息并等待下一行输入，不退出REPL

#### Scenario: 输入除零表达式
- **WHEN** 用户输入 "1/0"
- **THEN** 系统输出除零错误提示并等待下一行输入，不退出REPL

### Requirement: 空行处理
系统 SHALL 忽略空行输入，继续等待有效输入。

#### Scenario: 输入空行
- **WHEN** 用户输入空行
- **THEN** 系统不输出任何内容，继续等待下一行输入

### Requirement: 启动欢迎信息
系统 SHALL 在REPL启动时显示欢迎信息和退出提示。

#### Scenario: 启动应用
- **WHEN** 用户启动计算器应用
- **THEN** 系统显示欢迎信息及"输入quit或exit退出"提示
