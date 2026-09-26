## Why

当前 LLM 客户端的 reasoning effort（思考深度）是硬编码的，用户无法在交互过程中动态调整模型的推理深度。不同场景对思考深度的需求不同：简单问答不需要深度推理，复杂的多工具编排则需要模型充分思考。提供一个 `/effort` 命令让用户按需切换思考深度，可以平衡响应速度和推理质量。

## What Changes

- 在 CLI REPL 中新增 `/effort` 斜杠命令，支持 `none`、`low`、`high`、`max` 四种模式
- `/effort` 不带参数时显示当前 effort 模式
- `/effort <mode>` 切换到指定模式，默认值为 `high`
- `none` 模式下禁用 reasoning（不传 reasoning_effort 参数或传特殊值关闭推理）
- effort 设置传递到 LLM 客户端的 `chat` 和 `chat_stream` 方法，影响请求 payload
- `CockpitAgent` 持有当前 effort 状态，在每次 LLM 调用时传递给客户端

## Capabilities

### New Capabilities

- `effort-control`: 控制LLM推理深度的命令行交互能力，包括effort模式的定义、切换、传递和持久化

### Modified Capabilities

- `cli-interactive-chat`: REPL 斜杠命令列表新增 `/effort` 命令，用户交互流程中增加 effort 切换能力

## Impact

- `cli_repl.py`: 新增 `/effort` 命令解析和显示逻辑，欢迎信息更新
- `cockpit_agent.py`: `CockpitAgent` 增加 effort 属性，`chat` 和 `chat_stream` 传递 effort 参数
- `llm_client.py`: `OpenAICompatibleLLM` 和 `MockLLMClient` 的 `chat`/`chat_stream` 方法接收 effort 参数，在请求 payload 中设置 `reasoning_effort` 字段
- `config.py`: 新增 effort 模式常量定义和映射
- `main.py`: 可选地在启动参数中支持 `--effort` 覆盖默认值
