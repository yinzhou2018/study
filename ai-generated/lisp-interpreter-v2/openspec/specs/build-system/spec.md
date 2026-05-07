## Purpose

CMake build system configuration for the Lisp interpreter, integrating flex/bison code generation and managing dependencies.

## Requirements

### Requirement: CMake build with flex/bison integration
The build system SHALL use CMake 3.20+ and integrate flex/bison code generation as build steps.

#### Scenario: Full build from clean
- **WHEN** running `cmake -B build && cmake --build build` from the project root
- **THEN** flex and bison SHALL generate C++ source files, and the final `lisp` executable SHALL be produced

### Requirement: C++20 standard compilation
All C++ source files SHALL be compiled with C++20 standard.

#### Scenario: Verify C++20 features compile
- **WHEN** source code uses C++20 features (std::variant, concepts, ranges)
- **THEN** the build SHALL succeed without warnings

### Requirement: Discover flex and bison
The build system SHALL find flex and bison via `find_package` or `find_program` and report clear errors if not available.

#### Scenario: Flex not found
- **WHEN** flex is not installed on the system
- **THEN** CMake configuration SHALL fail with a clear message "flex not found"

#### Scenario: Bison not found
- **WHEN** bison is not installed on the system
- **THEN** CMake configuration SHALL fail with a clear message "bison not found"

### Requirement: File naming conventions
All header files SHALL use `.h` suffix and implementation files SHALL use `.cc` suffix, per project coding standards.

#### Scenario: Verify file extensions
- **WHEN** the build system processes source files
- **THEN** all headers SHALL end in `.h` and implementations SHALL end in `.cc`

### Requirement: Readline dependency
The build system SHALL find and link GNU readline when available, and provide a fallback without readline (using basic std::cin input).

#### Scenario: Readline available
- **WHEN** readline is installed
- **THEN** the build SHALL link readline and enable advanced REPL features

#### Scenario: Readline not available
- **WHEN** readline is not installed
- **THEN** the build SHALL succeed with basic line input (no history/editing)
