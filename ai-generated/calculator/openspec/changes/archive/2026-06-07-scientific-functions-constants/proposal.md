## Why

当前计算器仅支持四则运算，无法满足科学计算场景的基本需求。用户无法直接使用三角函数、对数、幂运算等常见数学函数，也无法引用 π、e 等数学常量，限制了计算器的实用性。

## What Changes

- 新增数学函数求值能力：支持 `sin`、`cos`、`tan`、`asin`、`acos`、`atan`、`sqrt`、`log`（以10为底）、`ln`（自然对数）、`abs`、`ceil`、`floor`、`pow` 等科学函数
- 新增数学常量支持：支持 `pi`（π）和 `e`（自然常数）作为命名常量在表达式中使用
- 扩展词法分析器以识别标识符（函数名和常量名）
- 扩展语法分析器以解析函数调用语法 `func(expr)`
- 扩展求值引擎以执行函数调用和常量替换

## Capabilities

### New Capabilities
- `scientific-functions`: 定义科学函数（三角、对数、幂、取整等）的求值能力，包括函数调用AST节点求值、参数合法性校验（如 `sqrt` 参数非负、`log`/`ln` 参数为正）
- `math-constants`: 定义数学命名常量（pi、e）的识别与求值能力，常量在求值时替换为对应的双精度浮点值

### Modified Capabilities
- `expression-parser`: 词法分析需新增标识符 Token 类型，语法分析需新增函数调用产生式 `FuncCall(identifier, expr)`
- `calculator-engine`: 求值引擎需支持 FuncCall AST 节点的求值，以及标识符节点的常量查找与替换

## Impact

- **源码变更**：词法分析器（新增标识符识别）、语法分析器（新增函数调用语法规则）、求值引擎（新增函数求值与常量查找）
- **依赖变更**：需链接 `<cmath>` 标准库头文件中的数学函数，无外部依赖新增
- **API 变更**：AST 节点类型新增 `FuncCallExpr` 和 `ConstantExpr`，表达式输入语法扩展
- **兼容性**：新增语法向后兼容，不影响已有四则运算表达式的解析与求值
