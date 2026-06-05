## Why

当前REPL交互模式没有输入提示符，用户无法直观判断程序是否在等待输入；同时输出结果带有"= "前缀，对交互体验而言冗余且不简洁。需要优化交互视觉体验，使其更符合常见交互式解释器的风格。

## What Changes

- 在REPL循环中添加 "> " 输入提示符，在终端交互模式下（isatty检测为真）显示
- 管道输入模式下不显示提示符，避免污染输出
- 移除输出结果前的 "= " 前缀，直接输出计算结果值
- 错误输出格式保持不变（"Error: ..."）
- 更新相关的单元测试和集成测试断言

## Capabilities

### New Capabilities

（无新增能力）

### Modified Capabilities

- `cli-interface`: 输出格式从 "= <结果>" 变更为 "<结果>"；REPL循环增加交互式输入提示符 "> "（仅终端模式）

## Impact

- `src/repl.cc`：添加提示符输出逻辑（isatty检测 + 输出"> "）、移除"= "前缀
- `src/repl.h`：可能需要增加isatty状态或终端检测相关成员
- `test/repl_test.cc`：更新断言从"= 3"到"3"
- `test/integration_test.cc`：更新断言从"= 3"到"3"
- 现有依赖 "= " 前缀的脚本或工具将受影响（**BREAKING**）
