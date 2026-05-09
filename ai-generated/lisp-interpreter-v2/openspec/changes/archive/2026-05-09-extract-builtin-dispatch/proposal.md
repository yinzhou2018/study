## Why

The evaluator module currently contains hard-coded dispatch logic for all built-in functions, violating separation of concerns. Adding a new builtin requires changes in three places: the handler function, the evaluator's dispatch logic, and the registration function. This coupling makes the code harder to maintain and extend. Extracting builtin dispatch to the builtins module creates a clean boundary where all builtin-related logic is self-contained.

## What Changes

- Create a `BuiltinManager` class in `builtins.h` that encapsulates all builtin registration and dispatch logic
- Move builtin dispatch from `Evaluator::ApplyFunction()` to `BuiltinManager::Call()`
- Simplify `Evaluator::RegisterBuiltins()` to delegate to `BuiltinManager::RegisterAll()`
- No functional changes - this is a pure refactoring for better architecture

## Capabilities

### New Capabilities
None (pure refactoring, no new capabilities)

### Modified Capabilities
None (requirement-level behavior unchanged - all existing evaluator and builtin specs remain valid)

## Impact

- **Files modified:** `builtins.h`, `builtins.cc`, `evaluator.cc`
- **Files unchanged:** `value.h`, `evaluator.h` (except possible minor cleanup)
- **Build system:** No changes required
- **Tests:** Existing tests should pass without modification
- **Extensibility:** Adding new builtins becomes a single-line operation in the builtins module
- **Testability:** `BuiltinManager` can be tested independently of the evaluator
