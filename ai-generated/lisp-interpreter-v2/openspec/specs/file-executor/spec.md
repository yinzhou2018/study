## Purpose

File execution mode for the Lisp interpreter, supporting batch evaluation of Lisp source files with error reporting.

## Requirements

### Requirement: Execute Lisp source files
The file executor SHALL read a `.lisp` file, parse all top-level expressions, and evaluate them sequentially.

#### Scenario: Execute a simple file
- **WHEN** the interpreter is invoked with `lisp test.lisp` where test.lisp contains `(define x 42)\n(+ x 1)`
- **THEN** all expressions SHALL be evaluated in order and the final result `43` SHALL be printed

#### Scenario: Multiple expressions in a file
- **WHEN** a file contains multiple top-level expressions
- **THEN** each expression SHALL be evaluated sequentially in the same environment

### Requirement: Report file errors
The file executor SHALL report meaningful errors when a file cannot be opened or contains syntax/runtime errors.

#### Scenario: File not found
- **WHEN** the interpreter is invoked with `lisp nonexistent.lisp`
- **THEN** it SHALL print an error message "file not found: nonexistent.lisp" and exit with non-zero status

#### Scenario: Runtime error in file
- **WHEN** a file contains an expression that causes a runtime error
- **THEN** the executor SHALL report the error with file name and line number, then stop execution

### Requirement: CLI invocation
The interpreter SHALL support `lisp` (no arguments -> REPL mode) and `lisp <file>` (file execution mode).

#### Scenario: No arguments starts REPL
- **WHEN** the interpreter is invoked with no arguments
- **THEN** it SHALL enter REPL mode

#### Scenario: Single argument executes file
- **WHEN** the interpreter is invoked with one argument
- **THEN** it SHALL execute the file and exit

#### Scenario: Too many arguments
- **WHEN** the interpreter is invoked with more than one argument
- **THEN** it SHALL print a usage message and exit with non-zero status
