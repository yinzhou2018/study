## Context

The Lisp interpreter uses CMake for building. Currently, CMakeLists.txt adds `${CMAKE_CURRENT_SOURCE_DIR}/include` to the include path, allowing source files to use `#include "xxx.h"` without directory prefixes. The project has 6 header files in `include/` and corresponding `.cc` implementations in `src/`.

For a small single-executable project, separating headers and implementations across two directories adds unnecessary complexity.

## Goals / Non-Goals

**Goals:**
- Consolidate all source files (headers and implementations) under `src/`
- Simplify project structure by eliminating the `include/` directory
- Maintain build compatibility without modifying source files

**Non-Goals:**
- Changing any functional behavior or APIs
- Modifying include statements in source code files
- Restructuring the internal organization of source files

## Decisions

### 1. Update Include Path in CMakeLists.txt

**Decision:** Change `target_include_directories(lisp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)` to use `src/` instead of `include/`.

**Rationale:** Since all source files currently use bare includes like `#include "evaluator.h"`, updating the include path is cleaner than modifying every source file. The compiler will find headers in `src/` just as it did in `include/`.

**Alternatives considered:**
- Modify all include statements to `#include "src/xxx.h"`: More invasive, changes more files, risk of typos
- Use `-I` flag directly in compile commands: Less maintainable, harder to understand

### 2. Move Files Without Modifying Content

**Decision:** Move header files from `include/` to `src/` using `git mv` to preserve file history.

**Rationale:** Git move maintains file history and makes the change easier to review. No content changes needed in the headers themselves.

### 3. Keep Generated Files Separate

**Decision:** Continue generating lexer and parser files in `CMAKE_CURRENT_BINARY_DIR` (current behavior).

**Rationale:** Generated files should remain separate from source files to avoid polluting the source tree and to support out-of-source builds.

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| IDE may lose track of include paths | Most IDEs auto-detect CMake include directories; may require project re-index |
| Existing scripts/tools may reference `include/` | Update any external references in documentation or tooling |
| Git history may be confusing | Using `git mv` preserves history as a rename operation |
| Include conflicts if other directories have same-named headers | Not applicable in this small project; all headers are unique to this codebase |

## Migration Plan

1. Move header files: `git mv include/*.h src/`
2. Update CMakeLists.txt include path
3. Delete empty `include/` directory: `rmdir include`
4. Verify build: `cmake --build build`
5. Run tests: `./build/lisp < test.lisp`
6. Commit changes

**Rollback:** Reverse the changes - move headers back, restore CMakeLists.txt.

## Open Questions

None identified. This is a straightforward refactoring with clear steps.
