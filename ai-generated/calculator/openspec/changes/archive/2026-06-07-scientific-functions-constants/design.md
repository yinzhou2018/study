## Context

当前计算器采用词法分析→语法分析→求值的三阶段管线架构，使用 `std::variant` 表达AST节点，`std::visit` 实现求值分发。现有AST仅有 `NumberExpr`、`BinaryExpr`、`UnaryExpr` 三种节点类型，词法单元仅有数字和运算符，无法表达函数调用或命名常量。

## Goals / Non-Goals

**Goals:**
- 支持以 `func(expr)` 形式调用科学函数（sin、cos、tan、asin、acos、atan、sqrt、log、ln、abs、ceil、floor、pow）
- 支持在表达式中使用命名常量 `pi` 和 `e`
- 函数参数域错误时返回明确的错误信息（如 `sqrt(-1)`、`log(0)`）
- 保持现有四则运算的完全向后兼容

**Non-Goals:**
- 不支持多参数函数（除 `pow(base, exp)` 外），不引入逗号分隔的参数列表语法
- 不支持自定义函数或变量赋值
- 不支持角度/弧度模式切换（统一使用弧度制）
- 不扩展到复数域

## Decisions

### 1. AST 新增 `FuncCallExpr` 和 `ConstantExpr` 节点

**选择**：在 `Expr` variant 中新增 `FuncCallExpr`（函数名 + 单参数表达式）和 `ConstantExpr`（常量名）两种节点。

**备选方案**：将常量视为零参数函数调用（如 `pi()` ），或用一个 `IdentifierExpr` 同时表示函数和常量。

**理由**：函数调用和常量引用是语义不同的概念——函数调用有参数需求值且有域约束，常量是无参的直接替换。分开表达更符合单一职责原则，且求值逻辑更清晰。`pow` 作为特殊双参数函数，采用专用 `PowExpr` 节点或在 `FuncCallExpr` 中使用 `vector<Expr>` 参数列表。

**结论**：`FuncCallExpr` 持有 `string name` 和 `vector<Expr> args`，支持单参数和双参数（`pow`）函数；`ConstantExpr` 持有 `string name`。这样不需要为 `pow` 单独创建AST节点。

### 2. 词法分析新增 `Identifier` Token 类型

**选择**：在 `TokenType` 枚举中新增 `Identifier`，值为标识符字符串（函数名或常量名）。

**备选方案**：为每个函数名和常量名单独创建 Token 类型（如 `Sin`、`Pi`）。

**理由**：单独创建 Token 会导致枚举膨胀且每次新增函数/常量都需修改枚举。统一用 `Identifier` + 字符串值，由语法分析阶段区分函数调用与常量引用，职责分离更合理。

**结论**：`Token` 结构体中 `string text` 字段仅在 `Identifier` 类型时有意义，复用现有 `double value` 字段语义不变。

### 3. 语法分析函数调用和常量的产生式

**选择**：在 `Factor` 产生式中新增规则：若当前 Token 为 `Identifier` 且下一个 Token 为 `LeftParen`，则解析为函数调用；否则解析为常量。

**备选方案**：在 `Primary` 层级单独增加产生式，或引入超前查看。

**理由**：函数调用与字面量和括号同属最高优先级，放在 `Factor` 中最自然。只需要查看下一个 Token（1-token 前瞻）即可区分函数调用与常量，对现有递归下降结构改动最小。

### 4. 求值阶段使用查找表映射函数名到 `<cmath>` 函数

**选择**：在求值器中使用 `unordered_map<string, FuncDef>` 查找表，将函数名映射到参数数量校验和对应的 `<cmath>` 调用。

**备选方案**：在求值的 `std::visit` 中用 if-else 链逐一判断函数名。

**理由**：查找表是数据驱动的，新增函数只需在表中添加一项，无需修改求值逻辑。符合开放-封闭原则，且便于测试和维护。

### 5. 域错误校验策略

**选择**：在求值器中，对每个函数调用检查参数域，不合法时返回 `Error`。

**规则**：
- `sqrt`：参数 ≥ 0
- `log`/`ln`：参数 > 0
- `asin`/`acos`：参数 ∈ [-1, 1]

**备选方案**：依赖 `<cmath>` 的 `NaN`/`errno` 机制，对结果检查 `std::isnan`。

**理由**：主动校验可以给出精确的错误信息（如"sqrt 的参数不能为负数"），而非返回晦涩的 "NaN" 结果。用户体验更好，且符合现有除零错误的处理模式。

## Risks / Trade-offs

- [FuncCallExpr 使用 vector<Expr> 参数列表] → 当前仅 `pow` 需要两参数，其余均为单参数。vector 带来少量堆分配开销，但统一了函数调用表示，避免为 `pow` 单独创建AST节点。可接受。
- [标识符大小写敏感] → 统一使用小写（`sin`、`pi`），大写输入将报错"未知标识符"。这是最简方案，避免模糊匹配的复杂性。
- [pow 函数引入逗号语法] → `pow(2,3)` 需要在词法分析中支持逗号 Token，语法分析中函数参数列表需处理逗号分隔。这是唯一的多参数函数，值得引入此机制以保持 `pow` 的自然语法。
