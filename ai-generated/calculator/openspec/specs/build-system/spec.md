# Build System

## Purpose

定义计算器项目的构建系统配置，包括CMake构建、编译、测试和格式化目标。

## Requirements

### Requirement: CMake项目配置
系统 SHALL 使用CMake作为构建系统，提供CMakeLists.txt配置文件，支持配置项目名称、C++20标准、源文件和头文件路径。

#### Scenario: 配置项目
- **WHEN** 执行 cmake -B build 命令
- **THEN** 系统成功生成构建配置，无报错

### Requirement: 编译目标
系统 SHALL 提供编译目标，将源代码编译为可执行的计算器应用。

#### Scenario: 编译项目
- **WHEN** 执行 cmake --build build 命令
- **THEN** 系统成功编译并生成calculator可执行文件

### Requirement: 测试目标
系统 SHALL 集成Google Test框架，提供CTest测试目标，支持运行所有单元测试。

#### Scenario: 运行测试
- **WHEN** 执行 ctest --test-dir build 命令
- **THEN** 系统运行所有单元测试并输出测试结果

### Requirement: 格式化目标
系统 SHALL 提供基于clang-format的代码格式化目标，确保代码风格统一。

#### Scenario: 格式化代码
- **WHEN** 执行 cmake --build build --target format 命令
- **THEN** 系统对所有C++源文件执行clang-format格式化

### Requirement: 跨平台支持
系统 SHALL 支持在macOS和Linux平台上编译和运行。

#### Scenario: macOS编译运行
- **WHEN** 在macOS平台执行 cmake -B build && cmake --build build
- **THEN** 系统成功编译并可正常执行calculator

#### Scenario: Linux编译运行
- **WHEN** 在Linux平台执行 cmake -B build && cmake --build build
- **THEN** 系统成功编译并可正常执行calculator
