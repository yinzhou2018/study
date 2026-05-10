## 1. Generalize body evaluation

- [x] 1.1 Add `EvalSequence(exprs, env)` helper in anonymous namespace that iterates a list, evaluates each car, returns last value

## 2. Fix cond

- [x] 2.1 Replace `EvalExpr(cp->cdr->car, env)` with `EvalSequence(cp->cdr, env)` in condition-matched branch
- [x] 2.2 Replace `EvalExpr(cp->cdr->car, env)` with `EvalSequence(cp->cdr, env)` in else branch

## 3. Fix let

- [x] 3.1 Replace `EvalExpr(body->car, let_env)` with `EvalSequence(body, let_env)`

## 4. Fix lambda

- [x] 4.1 In `EvalLambda` and `EvalDefine` shorthand, store full body list: `lambda->body = cell->cdr` instead of `lambda->body = body_cell->car`
- [x] 4.2 In `ApplyFunction`, replace `EvalExpr(lambda->body, call_env)` with `EvalSequence(lambda->body, call_env)`

## 5. Verify

- [x] 5.1 Build and run existing tests to confirm no regressions
- [x] 5.2 Test cond with multiple body expressions
- [x] 5.3 Test let with multiple body expressions
- [x] 5.4 Test lambda with multiple body expressions
