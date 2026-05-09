## 1. Implement BuiltinManager in builtins module

- [x] 1.1 Add BuiltinManager class declaration to builtins.h with public IsBuiltin(), Call(), and RegisterAll() methods
- [x] 1.2 Add private members to BuiltinManager: handlers_ map, Register() helper, and InitRegistry() function
- [x] 1.3 Implement InitRegistry() in builtins.cc to register all 17 builtin names with their handlers
- [x] 1.4 Implement BuiltinManager::IsBuiltin() to check if a name exists in handlers_
- [x] 1.5 Implement BuiltinManager::Call() to dispatch to the appropriate handler
- [x] 1.6 Implement BuiltinManager::RegisterAll() to initialize registry and register all builtins to environment

## 2. Refactor evaluator to use BuiltinManager

- [x] 2.1 Replace hard-coded dispatch in ApplyFunction() with BuiltinManager::IsBuiltin() and BuiltinManager::Call()
- [x] 2.2 Simplify RegisterBuiltins() to delegate to BuiltinManager::RegisterAll()

## 3. Verify Changes

- [x] 3.1 Clean and rebuild the project
- [x] 3.2 Verify build succeeds without errors or warnings
- [x] 3.3 Run test.lisp to verify all builtin operations work correctly

## 4. Finalize

- [x] 4.1 Review git diff to confirm changes match design
- [x] 4.2 Commit changes with descriptive message
