## Why

The project currently has a split directory structure with headers in `include/` and implementations in `src/`. For a small Lisp interpreter, this separation adds unnecessary complexity. Consolidating all source files under `src/` simplifies the project structure and reduces cognitive overhead.

## What Changes

- Move all 6 header files from `include/` to `src/`:
  - `builtins.h`
  - `environment.h`
  - `evaluator.h`
  - `parser.h`
  - `repl.h`
  - `value.h`
- Update all include paths in source files (`.cc`, `.l`, `.y`) from `#include "<name>.h"` to `#include "src/<name>.h"` or adjust include flags
- Delete the empty `include/` directory
- Update build system (CMakeLists.txt or Makefile) to reflect new source locations

## Capabilities

### New Capabilities
None (this is a refactoring change)

### Modified Capabilities
- `build-system`: Source file paths and include directories will change
- `evaluator`: Source file location changes (behavior unchanged)
- `lexer`: Source file location changes (behavior unchanged)
- `parser`: Source file location changes (behavior unchanged)
- `repl`: Source file location changes (behavior unchanged)

## Impact

- All `.cc`, `.l`, and `.y` files need their include statements updated
- Build configuration files need to be updated to point to `src/` for includes
- IDE project configurations (if any) may need include path updates
- No functional or API changes - pure refactoring
