# Specs for move-include-to-src Change

## No Requirement Changes

This change is a **pure refactoring** - it moves header files from `include/` to `src/` and updates the build configuration accordingly. 

**No functional requirements are changing:**

- The build system requirements remain the same (still uses CMake, flex, bison, C++20)
- The evaluator, lexer, parser, and REPL capabilities have identical functional behavior
- All existing test scenarios and acceptance criteria remain valid

**What is changing:**
- Physical file location: `include/*.h` → `src/*.h`
- Build configuration: include path in CMakeLists.txt changes from `include/` to `src/`

**What is NOT changing:**
- Any functional behavior or API contracts
- Requirement specifications in `openspec/specs/`
- Test scenarios or acceptance criteria

The delta specs are intentionally empty because this change affects only implementation details, not system requirements. All existing specs in `openspec/specs/` remain fully applicable without modification.
