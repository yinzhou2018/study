## 1. Token 与 AST 类型扩展

- [x] 1.1 在 TokenType 枚举中新增 Identifier 和 Comma 类型，在 Token 结构体中新增 text 字段用于标识符文本
- [x] 1.2 在 ast.h 中新增 FuncCallExpr（函数名 + 参数列表）和 ConstantExpr（常量名）节点类型，并将其加入 Expr variant

## 2. 词法分析器扩展

- [x] 2.1 在 lexer 中实现标识符识别逻辑：连续字母序列生成 Identifier Token，text 字段存储标识符字符串
- [x] 2.2 在 lexer 中实现逗号识别：生成 Comma Token
- [x] 2.3 编写词法分析单元测试：覆盖函数名、常量名、逗号、大写字母报错、标识符后接数字等场景

## 3. 语法分析器扩展

- [x] 3.1 在 Parser 中实现函数调用语法解析：Identifier 后跟 LeftParen 时解析参数列表，生成 FuncCallExpr
- [x] 3.2 在 Parser 中实现常量语法解析：Identifier 不跟 LeftParen 时生成 ConstantExpr
- [x] 3.3 编写语法分析单元测试：覆盖单参数函数、双参数函数、常量引用、运算优先级、括号不匹配等场景

## 4. 求值引擎扩展

- [x] 4.1 实现函数查找表：建立函数名到参数数量校验与 <cmath> 调用的映射，包含 sin/cos/tan/asin/acos/atan/sqrt/log/ln/abs/ceil/floor/pow
- [x] 4.2 实现函数参数域校验：sqrt 参数非负、log/ln 参数为正、asin/acos 参数在 [-1,1] 范围内，域错误返回明确错误信息
- [x] 4.3 实现函数参数数量校验：参数数量不匹配时返回错误信息
- [x] 4.4 实现未知函数校验：未注册函数名返回错误信息
- [x] 4.5 在 Evaluate 函数中新增 FuncCallExpr 和 ConstantExpr 的 std::visit 分支
- [x] 4.6 实现常量查找表：建立常量名到双精度浮点值的映射（pi、e），未知常量返回错误信息
- [x] 4.7 编写求值引擎单元测试：覆盖各函数求值、常量求值、域错误、参数数量错误、未知函数/常量等场景

## 5. 集成与代码规范审核

- [x] 5.1 更新 CMakeLists.txt 中测试源文件列表（如有新增测试文件）
- [x] 5.2 端到端测试：通过 REPL 输入科学函数和常量表达式验证完整管线
- [x] 5.3 代码规范审核：检查新增代码是否符合 Chromium C++ 编码规范，类成员变量不超过7个，函数不超过40行，头文件不暴露内部接口
