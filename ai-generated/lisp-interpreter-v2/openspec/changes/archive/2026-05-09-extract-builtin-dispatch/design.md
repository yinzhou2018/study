## Context

The current architecture mixes concerns: `Evaluator::ApplyFunction()` contains hard-coded if-else chains dispatching to `BuiltinArith()`, `BuiltinListOp()`, `BuiltinCompare()`, and `BuiltinBoolean()`. The evaluator knows the exact list of builtin names and which category each belongs to. Adding a new builtin requires:
1. Implementing the handler (in builtins.cc)
2. Adding dispatch logic in `ApplyFunction()`
3. Adding registration in `RegisterBuiltins()`

This violates separation of concerns and makes the evaluator module aware of implementation details that belong in the builtins module.

## Goals / Non-Goals

**Goals:**
- Encapsulate all builtin registration and dispatch logic within the builtins module
- Simplify the evaluator to delegate builtin handling without knowing builtin names
- Maintain existing functionality without behavioral changes
- Improve testability by isolating builtin logic

**Non-Goals:**
- Changing the Value type or adding new value variants
- Modifying existing builtin handler signatures
- Changing how special forms are evaluated
- Adding runtime plugin loading (deferred to future work)

## Decisions

### 1. BuiltinManager as Static Class

**Decision:** Create a static `BuiltinManager` class with `IsBuiltin()`, `Call()`, and `RegisterAll()` methods.

**Rationale:**
- Static methods provide a clean, stateless interface
- No need to instantiate or pass registry around
- Lazy initialization for the registry map ensures one-time setup
- Static fits the pattern of utility functions already present (e.g., `BuiltinArith`)

**Alternatives considered:**
- **Instance class with singleton:** Adds complexity for no benefit
- **Free functions in namespace:** Less encapsulated, harder to extend later
- **Function pointer map in evaluator:** Violates separation of concerns

### 2. Keep Existing Handler Functions

**Decision:** Maintain the category-based handler functions (`BuiltinArith`, `BuiltinListOp`, etc.) and register them as handlers for multiple names.

**Rationale:**
- Minimal code changes
- Handler functions are already well-tested
- Category grouping (arith, list, etc.) provides some organization
- Future refactoring to individual handlers can be done incrementally

**Alternatives considered:**
- **One handler per builtin:** More verbose now, but more extensible later
- **Single monolithic handler:** Would increase function size and reduce organization

### 3. Lazy Initialization with Function Static

**Decision:** Use a function-local static variable to initialize the registry on first call.

**Rationale:**
- One-time initialization guaranteed by C++
- No static initialization order issues
- Zero overhead if never called
- Clean separation of declaration and initialization

### 4. Evaluator Delegates via Symbol Lookup

**Decision:** In `ApplyFunction()`, when a Symbol is found, check `BuiltinManager::IsBuiltin()` and delegate to `BuiltinManager::Call()`.

**Rationale:**
- Evaluator still stores Symbols in the environment (unchanged behavior)
- Minimal change to evaluator's control flow
- Clear boundary: evaluator checks, builtins executes

**Alternatives considered:**
- **Builtin value variant:** Would change Value type, breaking existing code
- **Register as Lambda objects:** Would lose the distinction between builtins and user lambdas

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| Performance: Additional map lookup per builtin call | Map lookup is O(1) and negligible compared to evaluation overhead |
| Static initialization order issues | Use function-static variable to avoid static initialization fiasco |
| Test isolation: Global state in registry | Registry is read-only after initialization; tests can call `RegisterAll()` as needed |
| Future extensibility: Static class limits runtime registration | Can be refactored to instance class if runtime plugin loading is needed later |

## Migration Plan

1. Add `BuiltinManager` class declaration to `builtins.h`
2. Implement `BuiltinManager` in `builtins.cc` with:
   - Private `handlers_` map
   - Private `Register()` helper
   - Private `InitRegistry()` function
   - Public `IsBuiltin()`, `Call()`, `RegisterAll()`
3. Modify `evaluator.cc`:
   - Replace dispatch if-else chain in `ApplyFunction()` with `BuiltinManager::Call()`
   - Simplify `RegisterBuiltins()` to call `BuiltinManager::RegisterAll()`
4. Build and run tests to verify no behavioral changes
5. Commit changes

**Rollback:** Revert changes - the original code is fully functional.

## Open Questions

None identified. The design is straightforward and maintains backward compatibility.
