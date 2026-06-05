## 1. 项目结构与构建系统

- [x] 1.1 创建项目目录结构（src/, include/calculator/, test/）
- [x] 1.2 创建顶层CMakeLists.txt，配置项目名称、C++20标准、编译选项
- [x] 1.3 创建.clang-format配置文件，遵循Chromium代码风格
- [x] 1.4 集成Google Test，配置test/子目录的CMakeLists.txt
- [x] 1.5 添加clang-format格式化自定义目标
- [x] 1.6 验证项目可成功配置、编译、运行测试

## 2. 基础类型定义

- [x] 2.1 实现include/calculator/result.h：Result<T>模板及Error类型
- [x] 2.2 编写Result<T>单元测试，验证成功值和错误值的构造与访问
- [x] 2.3 实现include/calculator/token.h：Token类型及TokenType枚举
- [x] 2.4 编写Token类型单元测试
- [x] 2.5 实现include/calculator/ast.h：AST节点类型（NumberExpr, BinaryExpr, UnaryExpr），使用std::variant和std::unique_ptr
- [x] 2.6 编写AST节点类型单元测试

## 3. 词法分析器

- [x] 3.1 编写Lexer单元测试：解析数字、运算符、括号、一元负号
- [x] 3.2 编写Lexer错误处理测试：非法字符检测
- [x] 3.3 实现src/lexer.h和src/lexer.cc：Tokenize方法，逐字符扫描生成Token序列
- [x] 3.4 验证所有Lexer测试通过

## 4. 语法分析器

- [x] 4.1 编写Parser单元测试：解析加减表达式、乘除表达式、括号表达式
- [x] 4.2 编写Parser优先级测试：验证乘除优先于加减、括号改变优先级
- [x] 4.3 编写Parser错误处理测试：不匹配括号、不完整表达式
- [x] 4.4 实现src/parser.h和src/parser.cc：递归下降解析器，expr/term/factor文法规则
- [x] 4.5 验证所有Parser测试通过

## 5. 求值引擎

- [x] 5.1 编写Evaluator单元测试：四则运算求值、复合表达式求值
- [x] 5.2 编写Evaluator错误处理测试：除零检测
- [x] 5.3 实现src/evaluator.h和src/evaluator.cc：AST遍历求值，使用std::visit分发
- [x] 5.4 验证所有Evaluator测试通过

## 6. REPL交互界面

- [x] 6.1 编写Repl单元测试：表达式计算、退出命令、错误提示、空行处理
- [x] 6.2 实现src/repl.h和src/repl.cc：Read-Eval-Print循环逻辑
- [x] 6.3 实现src/main.cc：程序入口，启动REPL并显示欢迎信息
- [x] 6.4 验证所有Repl测试通过

## 7. 集成测试与质量验证

- [x] 7.1 编写端到端集成测试：从字符串输入到数值输出的完整流程
- [x] 7.2 运行全部单元测试和集成测试，确保通过率100%
- [x] 7.3 执行clang-format格式化检查，确保所有源文件符合编码规范
- [x] 7.4 代码审查：类成员变量不超过7个、函数不超过40行、无空类、头文件不暴露内部接口
- [x] 7.5 验证macOS平台编译运行正常
