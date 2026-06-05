## ADDED Requirements

### Requirement: 词法分析
系统 SHALL 将输入的数学表达式字符串分解为词法单元（Token）序列，支持数字（整数与浮点数）、运算符（+、-、*、/）、括号（(、)）。

#### Scenario: 解析简单加法表达式
- **WHEN** 输入表达式字符串 "1+2"
- **THEN** 系统输出词法单元序列 [Number(1), Plus, Number(2)]

#### Scenario: 解析浮点数表达式
- **WHEN** 输入表达式字符串 "3.14*2"
- **THEN** 系统输出词法单元序列 [Number(3.14), Star, Number(2)]

#### Scenario: 解析带括号表达式
- **WHEN** 输入表达式字符串 "(1+2)*3"
- **THEN** 系统输出词法单元序列 [LeftParen, Number(1), Plus, Number(2), RightParen, Star, Number(3)]

#### Scenario: 解析包含非法字符的表达式
- **WHEN** 输入表达式字符串 "1&2"
- **THEN** 系统 SHALL 抛出词法分析错误，指明非法字符的位置

### Requirement: 语法分析
系统 SHALL 将词法单元序列解析为抽象语法树（AST），正确处理运算符优先级（乘除高于加减）和括号嵌套。

#### Scenario: 解析加法表达式生成AST
- **WHEN** 输入词法单元序列 [Number(1), Plus, Number(2)]
- **THEN** 系统生成AST节点 AddExpr(NumberExpr(1), NumberExpr(2))

#### Scenario: 运算符优先级处理
- **WHEN** 输入词法单元序列 [Number(1), Plus, Number(2), Star, Number(3)]
- **THEN** 系统生成AST节点 AddExpr(NumberExpr(1), MulExpr(NumberExpr(2), NumberExpr(3)))

#### Scenario: 括号改变优先级
- **WHEN** 输入词法单元序列 [LeftParen, Number(1), Plus, Number(2), RightParen, Star, Number(3)]
- **THEN** 系统生成AST节点 MulExpr(AddExpr(NumberExpr(1), NumberExpr(2)), NumberExpr(3))

#### Scenario: 解析不匹配的括号
- **WHEN** 输入词法单元序列 [LeftParen, Number(1), Plus, Number(2)]
- **THEN** 系统 SHALL 抛出语法分析错误，指明括号不匹配

### Requirement: 一元负号支持
系统 SHALL 支持一元负号运算符，用于表示负数。

#### Scenario: 负数字面量
- **WHEN** 输入表达式字符串 "-5"
- **THEN** 系统成功解析并生成包含一元负号的AST

#### Scenario: 表达式中的负号
- **WHEN** 输入表达式字符串 "3*-2"
- **THEN** 系统成功解析并生成包含一元负号的AST
