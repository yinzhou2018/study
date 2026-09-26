## Why

当前系统仅支持批量演示模式运行（`main.py` 中硬编码查询列表），无交互式对话入口；LLM 调用为阻塞式同步请求，无法流式展示思考过程和工具调用进度；用户无法在生成过程中打断当前操作。需要增加命令行交互式对话支持，让用户能实时与 Agent 对话、观察过程并随时打断。

## What Changes

- 新增命令行交互式 REPL 入口，支持多轮对话输入，用户可连续提问、退出、查看历史
- LLM 客户端增加流式响应（SSE streaming）支持，逐 token 输出思考内容与最终回复
- 工具调用过程实时展示：工具名称、参数、执行状态、返回结果，以可读格式流式打印
- 支持 Esc 键打断当前 LLM 生成或工具执行，打断后保留已完成部分、回到对话提示符
- 会话管理：支持 `/exit`、`/clear`（清空对话历史）、`/history`（查看消息历史）等命令
- 保留原有批量演示模式，新增 `--interactive` 启动参数切换至交互模式

## Capabilities

### New Capabilities
- `cli-interactive-chat`: 命令行交互式对话 REPL，包含多轮输入、流式输出渲染、打断处理、会话命令管理

### Modified Capabilities
<!-- 无现有 spec 需要修改 -->

## Impact

- **llm_client.py**: `OpenAICompatibleLLM` 增加 `chat_stream` 方法支持 SSE 流式解析；`MockLLMClient` 增加流式模拟方法
- **cockpit_agent.py**: `CockpitAgent.chat` 重构为支持流式回调（逐 token 回调 + 工具调用事件回调），支持打断信号
- **main.py**: 新增 `--interactive` 参数分支，启动 REPL 循环
- **新增 `cli_repl.py`**: 交互式 REPL 逻辑，输入解析、流式渲染、Esc 键打断处理
- **requirements.txt**: 可能新增流式解析依赖（如无则使用标准库）
- 无外部 API 变更，不影响现有工具集/网关逻辑
