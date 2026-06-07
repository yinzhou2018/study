## ADDED Requirements

### Requirement: 命名常量识别与求值
系统 SHALL 支持在表达式中使用命名常量 pi（圆周率 π）和 e（自然常数），求值时将常量名替换为对应的双精度浮点数值。

#### Scenario: pi 常量求值
- **WHEN** 输入AST节点 ConstantExpr("pi")
- **THEN** 系统返回计算结果 3.141592653589793

#### Scenario: e 常量求值
- **WHEN** 输入AST节点 ConstantExpr("e")
- **THEN** 系统返回计算结果 2.718281828459045

#### Scenario: 常量参与运算
- **WHEN** 输入对应表达式 "2*pi" 的AST
- **THEN** 系统返回计算结果 6.283185307179586

#### Scenario: 常量作为函数参数
- **WHEN** 输入对应表达式 "sin(pi)" 的AST
- **THEN** 系统返回计算结果 0（允差 1e-9）

### Requirement: 未知常量校验
系统 SHALL 对未注册的常量名返回错误信息。

#### Scenario: 引用未定义的常量
- **WHEN** 输入AST节点 ConstantExpr("unknown")
- **THEN** 系统 SHALL 返回错误信息，指明未知常量 unknown
