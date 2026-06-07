## ADDED Requirements

### Requirement: FuncCallExpr 节点求值
系统 SHALL 支持 FuncCallExpr AST 节点的求值，通过函数名查找对应的计算函数并执行。函数名与计算函数的映射由查找表维护。

#### Scenario: 求值单参数函数调用
- **WHEN** 输入AST节点 FuncCallExpr("sin", [NumberExpr(1.5707963267948966)])
- **THEN** 系统通过查找表找到 sin 对应的计算函数，求值参数后执行并返回结果 1（允差 1e-9）

#### Scenario: 求值双参数函数调用
- **WHEN** 输入AST节点 FuncCallExpr("pow", [NumberExpr(2), NumberExpr(3)])
- **THEN** 系统通过查找表找到 pow 对应的计算函数，依次求值两个参数后执行并返回结果 8

### Requirement: ConstantExpr 节点求值
系统 SHALL 支持 ConstantExpr AST 节点的求值，通过常量名查找对应的双精度浮点值。

#### Scenario: 求值 pi 常量
- **WHEN** 输入AST节点 ConstantExpr("pi")
- **THEN** 系统返回 3.141592653589793

#### Scenario: 求值 e 常量
- **WHEN** 输入AST节点 ConstantExpr("e")
- **THEN** 系统返回 2.718281828459045

#### Scenario: 求值未知常量
- **WHEN** 输入AST节点 ConstantExpr("unknown")
- **THEN** 系统 SHALL 返回错误信息，指明未知常量 unknown
