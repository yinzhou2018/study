## 1. 提取表达式求值函数

- [x] 1.1 在 `evaluator.h/.cc` 中新增自由函数 `EvaluateExpression(const std::string&)`，封装 Lexer→Parser→Evaluator 管线，返回 `Result<double>`
- [x] 1.2 重构 `Repl::ProcessLine()` 调用 `EvaluateExpression()` 替代内联管线代码，保持 REPL 输出行为不变
- [x] 1.3 补充 `EvaluateExpression` 单元测试：正常表达式、词法错误、语法错误、除零错误

## 2. 修改主入口支持命令行参数

- [x] 2.1 修改 `main.cc`：解析 `argc/argv`，存在参数时将 `argv[1..]` 用空格拼接为表达式字符串
- [x] 2.2 命令行参数模式调用 `EvaluateExpression()`，结果输出到 stdout，以退出码 0 退出
- [x] 2.3 命令行参数模式求值失败时，错误信息输出到 stderr，以退出码 1 退出
- [x] 2.4 无参数时保持现有 `Repl` 行为不变

## 3. 测试与验证

- [x] 3.1 补充集成测试：单参数表达式执行、多参数拼接执行、错误退出码验证
- [x] 3.2 验证无参数启动 REPL 行为无回归
- [x] 3.3 验证管道模式行为无回归

## 4. 代码规范审核

- [x] 4.1 审核新增代码符合 Chromium C++ 编码规范：头文件后缀 `.h`、实现文件后缀 `.cc`、无超过 40 行函数、无超过 7 个成员变量的类
- [x] 4.2 运行 `cmake --build build --target format` 格式化检查
