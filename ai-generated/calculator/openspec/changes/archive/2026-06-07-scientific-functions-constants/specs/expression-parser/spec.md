## ADDED Requirements

### Requirement: 标识符词法分析
系统 SHALL 将字母序列识别为标识符 Token（Identifier），用于表示函数名和常量名。标识符由一个或多个英文字母组成，大小写敏感（仅识别小写）。

#### Scenario: 解析函数名
- **WHEN** 输入表达式字符串 "sin(1)"
- **THEN** 系统输出词法单元序列包含 Identifier("sin")

#### Scenario: 解析常量名
- **WHEN** 输入表达式字符串 "pi"
- **THEN** 系统输出词法单元序列 [Identifier("pi"), EndOfInput]

#### Scenario: 解析大写标识符报错
- **WHEN** 输入表达式字符串 "Sin(1)"
- **THEN** 系统 SHALL 抛出词法分析错误，指明非法字符 'S' 的位置

#### Scenario: 标识符后接数字
- **WHEN** 输入表达式字符串 "pi2"
- **THEN** 系统输出词法单元序列 [Identifier("pi"), Number(2), EndOfInput]

### Requirement: 逗号词法分析
系统 SHALL 将逗号字符 `,` 识别为 Comma Token，用于函数多参数分隔。

#### Scenario: 解析逗号
- **WHEN** 输入表达式字符串 "pow(2,3)"
- **THEN** 系统输出词法单元序列包含 Identifier("pow"), LeftParen, Number(2), Comma, Number(3), RightParen

### Requirement: 函数调用语法分析
系统 SHALL 支持函数调用语法 `identifier(arg1, arg2, ...)`，在 Factor 产生式中解析。当标识符后跟左括号时，解析为函数调用；否则解析为常量引用。

#### Scenario: 解析单参数函数调用
- **WHEN** 输入词法单元序列 [Identifier("sin"), LeftParen, Number(1.5707963267948966), RightParen]
- **THEN** 系统生成AST节点 FuncCallExpr("sin", [NumberExpr(1.5707963267948966)])

#### Scenario: 解析双参数函数调用
- **WHEN** 输入词法单元序列 [Identifier("pow"), LeftParen, Number(2), Comma, Number(3), RightParen]
- **THEN** 系统生成AST节点 FuncCallExpr("pow", [NumberExpr(2), NumberExpr(3)])

#### Scenario: 解析函数调用优先级
- **WHEN** 输入表达式字符串 "2*sin(1)"
- **THEN** 系统生成AST节点 MulExpr(NumberExpr(2), FuncCallExpr("sin", [NumberExpr(1)]))

#### Scenario: 函数调用中括号不匹配
- **WHEN** 输入词法单元序列 [Identifier("sin"), LeftParen, Number(1)]
- **THEN** 系统 SHALL 抛出语法分析错误，指明括号不匹配

### Requirement: 常量语法分析
系统 SHALL 将不跟左括号的标识符解析为常量引用，生成 ConstantExpr AST 节点。

#### Scenario: 解析常量
- **WHEN** 输入词法单元序列 [Identifier("pi")]
- **THEN** 系统生成AST节点 ConstantExpr("pi")

#### Scenario: 解析常量参与运算
- **WHEN** 输入表达式字符串 "2*pi"
- **THEN** 系统生成AST节点 MulExpr(NumberExpr(2), ConstantExpr("pi"))
