## Context

当前 `EvalCond`、`EvalLet`、`EvalLambda` + `ApplyFunction` 在求值 body 时都只处理第一个表达式。但在 Lisp 语义中，`cond` 子句、`let`、`lambda` 的 body 都具备隐式 `begin` 语义：依次求值所有表达式，返回最后一个的值。

影响范围：`src/evaluator.cc`，三处 body 求值逻辑 + lambda body 存储方式。

## Goals / Non-Goals

**Goals:**
- `cond` 匹配分支的所有表达式依次求值，返回最后一个
- `let` body 的所有表达式依次求值，返回最后一个
- `lambda` body 的所有表达式依次求值，返回最后一个
- 提取可复用的序列求值辅助函数

**Non-Goals:**
- 不改变 `begin` 是否为独立特殊形式（当前没有 `begin`，也不在此次引入）
- 不改变其他特殊形式的求值语义

## Decisions

### 提取 `EvalSequence` 辅助函数

在 `evaluator.cc` 匿名 namespace 中新增 `EvalSequence(Value exprs, std::shared_ptr<Environment> env) -> Value`：
- 遍历链表 `exprs`，依次对每个 pair 的 car 调用 `EvalExpr`
- 跟踪最后一个结果，在遇到 Nil 时返回
- 如果列表为空（直接传 Nil），返回 Nil

**替代方案**：在每个调用处内联 while 循环 → 拒绝，重复三遍的代码不如一个 helper。

### `EvalCond` 修改

两处需要改：
1. 普通条件匹配（第167-168行）：当前 `EvalExpr(cp->cdr->car, env)` 改为 `EvalSequence(cp->cdr, env)`
2. else 分支（第164-165行）：同上

### `EvalLet` 修改

第189行：当前 `EvalExpr(body->car, let_env)` 改为 `EvalSequence(body, let_env)`。

由于 body 本身是 `Pair` 链表，不需要额外解析。

### `EvalLambda` + `ApplyFunction` 修改

`EvalLambda`（第205行）：将 `lambda->body = body_cell->car` 改为 `lambda->body = body_cell`（存储整个 body 列表，而非只 car）。

`ApplyFunction`（第73行）：将 `EvalExpr(lambda->body, call_env)` 改为 `EvalSequence(lambda->body, call_env)`。

`EvalSequence` 签名与 `EvalExpr` 不同（接受 `Value` 链表 + `Environment`），所以此处需要显式调用 helper 而非走 `EvalExpr` 主逻辑。

### 为何不改 Lambda 的 body 类型

保持 `Lambda::body` 为 `Value` 类型不变。body 列表本身就是一个 `std::shared_ptr<Pair>` 或 `Nil`，已在 `Value` 的 variant 范围内。唯一的语义变化是：调用时不再把整个 body 当作单个表达式求值，而是用 `EvalSequence` 遍历求值。

## Risks / Trade-offs

- **Lambda body 为空时**：`EvalSequence` 遇到空序列返回 Nil，符合 Scheme 语义
- **兼容性**：现有代码中传单表达式的情况不受影响，`EvalSequence` 对单元素列表行为等同于之前的 `EvalExpr`（求值后返回）
- **性能**：单表达式场景增加一次循环判断开销，可以忽略不计
