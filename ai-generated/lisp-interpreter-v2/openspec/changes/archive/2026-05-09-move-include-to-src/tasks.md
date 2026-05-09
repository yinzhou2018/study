## 1. Move Header Files

- [x] 1.1 Move builtins.h from include/ to src/ using git mv
- [x] 1.2 Move environment.h from include/ to src/ using git mv
- [x] 1.3 Move evaluator.h from include/ to src/ using git mv
- [x] 1.4 Move parser.h from include/ to src/ using git mv
- [x] 1.5 Move repl.h from include/ to src/ using git mv
- [x] 1.6 Move value.h from include/ to src/ using git mv
- [x] 1.7 Delete the empty include/ directory using rmdir

## 2. Update Build Configuration

- [x] 2.1 Edit CMakeLists.txt to change include path from `${CMAKE_CURRENT_SOURCE_DIR}/include` to `${CMAKE_CURRENT_SOURCE_DIR}/src`

## 3. Verify Build

- [x] 3.1 Clean build directory: rm -rf build && cmake -B build
- [x] 3.2 Build project: cmake --build build
- [x] 3.3 Verify build succeeds without errors
- [x] 3.4 Run basic test: ./build/lisp < test.lisp

## 4. Finalize

- [x] 4.1 Review git diff to confirm changes are as expected
- [x] 4.2 Commit changes with descriptive message
