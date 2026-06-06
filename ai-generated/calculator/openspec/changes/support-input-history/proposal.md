## Why

当前 REPL 使用 `std::getline` 读取输入，不支持行编辑和输入历史导航。用户无法通过上下方向键浏览之前输入的表达式，也无法跨会话保留历史记录，交互体验较差。

## What Changes

- 引入 linenoise 库替代 `std::getline`，提供行编辑和历史导航能力
- 在 REPL 会话内支持上下方向键浏览已输入的历史表达式
- 支持跨会话历史持久化，将历史记录保存至用户目录下的文件中
- 非交互模式（管道/重定向输入）保持原有 `std::getline` 行为不变

## Capabilities

### New Capabilities
- `input-history`: 输入历史管理，包括会话内历史存储、方向键导航、跨会话持久化

### Modified Capabilities
- `cli-interface`: REPL 输入方式从 `std::getline` 切换为 linenoise，交互模式行为变更

## Impact

- `src/repl.h` / `src/repl.cc`: Repl 类需引入 linenoise 输入接口，增加历史文件路径配置
- `CMakeLists.txt` / `src/CMakeLists.txt`: 添加 linenoise 作为依赖（使用 FetchContent 或 git submodule）
- `test/repl_test.cc`: 需适配新的输入接口
- 新增 linenoise 第三方库依赖
