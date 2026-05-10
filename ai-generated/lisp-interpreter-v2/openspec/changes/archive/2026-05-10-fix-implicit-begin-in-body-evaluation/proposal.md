## Why

`cond`、`let`、`lambda` 的 body 具有隐式 `begin` 语义（多个表达式依次求值，返回最后一个的值）。当前实现只求值了 body 的第一个表达式，导致多表达式 body 中的副作用丢失、非首个表达式的计算结果被忽略。

## What Changes

- 提取 `EvalSequence` 辅助函数，接收表达式列表和环境，依次求值并返回最后一个结果
- 修复 `EvalCond`：条件匹配分支和 else 分支的 body 改为使用 `EvalSequence`
- 修复 `EvalLet`：body 改为使用 `EvalSequence`，以支持隐式 `begin`
- 修复 `EvalLambda` + `ApplyFunction`：lambda body 存储完整 body 列表，调用时使用 `EvalSequence`
- 更新 evaluator spec 中 cond、let、lambda 的 requirement，增加多表达式 body 的场景

## Capabilities

### New Capabilities

（无）

### Modified Capabilities

- `evaluator`: cond 条件分支、let body、lambda body 的求值语义从"只求值第一个表达式"改为"依次求值所有表达式，返回最后一个的值"

## Impact

- `src/evaluator.cc` — `EvalCond`, `EvalLet`, `EvalLambda`, `ApplyFunction` 修改；新增 `EvalSequence` 辅助函数
