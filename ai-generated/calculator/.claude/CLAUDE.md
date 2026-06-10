# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在本仓库中工作时提供指导。

## 构建与运行命令

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release   # 配置
cmake --build build                          # 构建
./build/src/calculator                       # 运行（交互式 REPL）
echo "1+2" | ./build/src/calculator          # 运行（管道输入）
ctest --test-dir build                       # 运行所有测试
./build/test/calculator_test                 # 直接运行测试
./build/test/calculator_test --gtest_filter=LexerTest*  # 运行单个测试套件
cmake --build build --target format          # clang-format 格式化所有源/测试文件
```

## 架构

线性流水线，支持依赖注入——每个阶段独立测试：

```
输入字符串 → InputReader → Lexer → Parser → Evaluator → Repl
```

- **InputReader**（`input_reader.h/.cc`）：抽象基类，两个实现——`StdinReader`（管道/测试）和 `LinenoiseReader`（交互式，历史记录持久化于 `~/.calculator_history`）
- **Lexer**（`lexer.h/.cc`）：单趟字符→词法单元转换器。标识符仅支持小写；大写字母会触发错误
- **Parser**（`parser.h/.cc`）：递归下降解析器，生成具有正确运算符优先级（`*/` > `+-`）的 AST。文法：Expr→Term→Factor，支持一元取负、括号分组、函数调用（逗号分隔参数）和常量引用
- **Evaluator**（`evaluator.h/.cc`）：自由函数 `Evaluate()`，使用 `std::visit` 对 AST 变体进行访问——无类、无虚函数分发。静态查找表支持 13 个科学函数和 2 个常量（pi、e）
- **Repl**（`repl.h/.cc`）：编排流水线。构造函数通过 `isatty()` 自动选择输入模式。接受注入的 `InputReader`+`ostream` 用于测试

关键设计选择：基于 `std::variant` 的 AST（无虚函数），`unique_ptr` 间接引用递归节点，`Result<T>` 错误传播（无异常），函数式求值器风格。
