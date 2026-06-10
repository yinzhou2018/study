## MODIFIED Requirements

### Requirement: REPL交互模式
系统 SHALL 提供REPL（Read-Eval-Print Loop）交互模式，逐行读取用户输入的数学表达式，计算并直接输出结果（不带前缀）。在终端交互模式下，系统 SHALL 使用 linenoise 库提供行编辑和历史导航功能，并在等待输入前显示 "> " 提示符；在管道输入模式下，系统 SHALL 使用 `std::getline` 读取输入且不显示提示符。当命令行存在参数时，系统 SHALL 不进入 REPL 模式，转而执行命令行参数表达式。

#### Scenario: 交互式计算
- **WHEN** 用户在终端REPL中输入 "1+2" 并按回车
- **THEN** 系统输出 "3"（不带"= "前缀）并显示 "> " 提示符等待下一行输入

#### Scenario: 连续多次计算
- **WHEN** 用户依次在终端REPL中输入 "2+3" 和 "4*5"
- **THEN** 系统依次输出 "5" 和 "20"（均不带"= "前缀）

#### Scenario: 管道输入模式
- **WHEN** 通过管道输入表达式（如 `echo "1+2" | calculator`）
- **THEN** 系统输出 "3"（不带"= "前缀）且不显示 "> " 提示符

#### Scenario: 交互模式行编辑
- **WHEN** 用户在终端REPL中输入时使用退格键、方向键
- **THEN** 系统正确响应行编辑操作和历史导航

#### Scenario: 命令行参数时跳过REPL
- **WHEN** 用户运行 `calculator "1+2"`（带命令行参数）
- **THEN** 系统不进入 REPL 模式，直接执行表达式后退出
