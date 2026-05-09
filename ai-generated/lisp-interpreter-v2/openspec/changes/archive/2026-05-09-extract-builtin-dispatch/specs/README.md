# Specs for extract-builtin-dispatch Change

## No Requirement Changes

This change is a **pure refactoring** - it extracts builtin dispatch logic from the evaluator module into the builtins module for better separation of concerns.

**No functional requirements are changing:**

- All existing builtin operations (`+`, `-`, `*`, `/`, `car`, `cdr`, `cons`, `list`, `null?`, `pair?`, `=`, `<`, `>`, `<=`, `>=`, `not`) work identically
- Special forms (`define`, `if`, `cond`, `let`, `quote`, `lambda`, `and`, `or`) are unchanged
- Environment scoping and lambda closures work identically
- Error messages and error conditions remain the same

**What is changing:**
- **Implementation only:** Builtin dispatch logic moves from `evaluator.cc` to `builtins.cc`
- **Internal API:** `BuiltinManager` class added to `builtins.h`
- **Evaluator coupling:** `evaluator.cc` no longer knows specific builtin names

**What is NOT changing:**
- Any functional behavior or API contracts visible to Lisp code
- Requirement specifications in `openspec/specs/evaluator/spec.md`
- Test scenarios and acceptance criteria
- Value types or representation

The delta specs are intentionally empty because this change affects only internal implementation details, not system requirements. All existing specs in `openspec/specs/` remain fully applicable without modification.
