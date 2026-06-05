# Calculator Engine

## Purpose

定义计算器核心求值引擎的能力，包括四则运算、除零检测、浮点数精度和复合表达式求值。

## Requirements

### Requirement: 四则运算求值
系统 SHALL 对抽象语法树执行求值运算，支持加、减、乘、除四种基本运算。

#### Scenario: 加法求值
- **WHEN** 输入AST节点 AddExpr(NumberExpr(1), NumberExpr(2))
- **THEN** 系统返回计算结果 3

#### Scenario: 减法求值
- **WHEN** 输入AST节点 SubExpr(NumberExpr(5), NumberExpr(3))
- **THEN** 系统返回计算结果 2

#### Scenario: 乘法求值
- **WHEN** 输入AST节点 MulExpr(NumberExpr(4), NumberExpr(3))
- **THEN** 系统返回计算结果 12

#### Scenario: 除法求值
- **WHEN** 输入AST节点 DivExpr(NumberExpr(10), NumberExpr(2))
- **THEN** 系统返回计算结果 5

### Requirement: 除零检测
系统 SHALL 在除法运算的除数为零时返回错误，而非产生未定义行为。

#### Scenario: 除以零
- **WHEN** 输入AST节点 DivExpr(NumberExpr(1), NumberExpr(0))
- **THEN** 系统 SHALL 返回除零错误，不产生崩溃或未定义行为

### Requirement: 浮点数精度
系统 SHALL 使用双精度浮点数（double）进行计算，并正确处理浮点数运算结果。

#### Scenario: 浮点数乘法
- **WHEN** 输入AST节点 MulExpr(NumberExpr(3.14), NumberExpr(2))
- **THEN** 系统返回计算结果 6.28

### Requirement: 复合表达式求值
系统 SHALL 能对包含多种运算符的复合AST正确求值。

#### Scenario: 混合运算求值
- **WHEN** 输入对应表达式 "(1+2)*3" 的AST
- **THEN** 系统返回计算结果 9
