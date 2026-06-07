# Scientific Functions

## Purpose

定义科学计算函数的求值能力，包括单参数函数、双参数幂函数、参数域校验、参数数量校验及未知函数校验。

## Requirements

### Requirement: 单参数科学函数求值
系统 SHALL 支持以下单参数科学函数的求值：sin、cos、tan、asin、acos、atan、sqrt、log（以10为底的对数）、ln（自然对数）、abs、ceil、floor。所有三角函数使用弧度制。

#### Scenario: sin 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("sin", [NumberExpr(1.5707963267948966)])
- **THEN** 系统返回计算结果 1（允差 1e-9）

#### Scenario: cos 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("cos", [NumberExpr(0)])
- **THEN** 系统返回计算结果 1

#### Scenario: tan 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("tan", [NumberExpr(0.7853981633974483)])
- **THEN** 系统返回计算结果 1（允差 1e-9）

#### Scenario: asin 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("asin", [NumberExpr(0.5)])
- **THEN** 系统返回计算结果 0.5235987755982989（允差 1e-9）

#### Scenario: acos 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("acos", [NumberExpr(0.5)])
- **THEN** 系统返回计算结果 1.0471975511965979（允差 1e-9）

#### Scenario: atan 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("atan", [NumberExpr(1)])
- **THEN** 系统返回计算结果 0.7853981633974483（允差 1e-9）

#### Scenario: sqrt 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("sqrt", [NumberExpr(4)])
- **THEN** 系统返回计算结果 2

#### Scenario: log 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("log", [NumberExpr(100)])
- **THEN** 系统返回计算结果 2

#### Scenario: ln 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("ln", [NumberExpr(2.718281828459045)])
- **THEN** 系统返回计算结果 1（允差 1e-9）

#### Scenario: abs 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("abs", [NumberExpr(-3.5)])
- **THEN** 系统返回计算结果 3.5

#### Scenario: ceil 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("ceil", [NumberExpr(3.2)])
- **THEN** 系统返回计算结果 4

#### Scenario: floor 函数求值
- **WHEN** 输入AST节点 FuncCallExpr("floor", [NumberExpr(3.8)])
- **THEN** 系统返回计算结果 3

### Requirement: 双参数幂函数求值
系统 SHALL 支持 pow(base, exponent) 双参数函数的求值，计算 base 的 exponent 次幂。

#### Scenario: pow 正整数幂
- **WHEN** 输入AST节点 FuncCallExpr("pow", [NumberExpr(2), NumberExpr(3)])
- **THEN** 系统返回计算结果 8

#### Scenario: pow 负数幂
- **WHEN** 输入AST节点 FuncCallExpr("pow", [NumberExpr(4), NumberExpr(0.5)])
- **THEN** 系统返回计算结果 2

### Requirement: 函数参数域校验
系统 SHALL 在求值前校验函数参数的合法性，对域外参数返回明确的错误信息。

#### Scenario: sqrt 负数参数
- **WHEN** 输入AST节点 FuncCallExpr("sqrt", [NumberExpr(-1)])
- **THEN** 系统 SHALL 返回错误信息，指明 sqrt 的参数不能为负数

#### Scenario: log 零参数
- **WHEN** 输入AST节点 FuncCallExpr("log", [NumberExpr(0)])
- **THEN** 系统 SHALL 返回错误信息，指明 log 的参数必须为正数

#### Scenario: log 负数参数
- **WHEN** 输入AST节点 FuncCallExpr("log", [NumberExpr(-1)])
- **THEN** 系统 SHALL 返回错误信息，指明 log 的参数必须为正数

#### Scenario: ln 零参数
- **WHEN** 输入AST节点 FuncCallExpr("ln", [NumberExpr(0)])
- **THEN** 系统 SHALL 返回错误信息，指明 ln 的参数必须为正数

#### Scenario: asin 超出范围参数
- **WHEN** 输入AST节点 FuncCallExpr("asin", [NumberExpr(2)])
- **THEN** 系统 SHALL 返回错误信息，指明 asin 的参数必须在 -1 到 1 之间

#### Scenario: acos 超出范围参数
- **WHEN** 输入AST节点 FuncCallExpr("acos", [NumberExpr(-1.5)])
- **THEN** 系统 SHALL 返回错误信息，指明 acos 的参数必须在 -1 到 1 之间

### Requirement: 函数参数数量校验
系统 SHALL 校验函数调用时的参数数量，参数数量不匹配时返回错误。

#### Scenario: 单参数函数传入零个参数
- **WHEN** 输入AST节点 FuncCallExpr("sin", [])
- **THEN** 系统 SHALL 返回错误信息，指明 sin 需要 1 个参数

#### Scenario: 单参数函数传入两个参数
- **WHEN** 输入AST节点 FuncCallExpr("sin", [NumberExpr(1), NumberExpr(2)])
- **THEN** 系统 SHALL 返回错误信息，指明 sin 需要 1 个参数

#### Scenario: pow 传入一个参数
- **WHEN** 输入AST节点 FuncCallExpr("pow", [NumberExpr(2)])
- **THEN** 系统 SHALL 返回错误信息，指明 pow 需要 2 个参数

### Requirement: 未知函数校验
系统 SHALL 对未注册的函数名返回错误信息。

#### Scenario: 调用未定义的函数
- **WHEN** 输入AST节点 FuncCallExpr("unknown", [NumberExpr(1)])
- **THEN** 系统 SHALL 返回错误信息，指明未知函数 unknown
