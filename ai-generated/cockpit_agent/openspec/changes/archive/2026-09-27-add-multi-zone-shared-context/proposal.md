++ Add File: openspec/changes/add-multi-zone-shared-context/proposal.md
## Why

智能座舱内存在多个物理音区和多名乘客，但当前 REPL 和 Agent 只维护单一用户视角的对话历史，无法表达“谁在哪个音区说话”，也不能让模型在共享上下文中自行判断跨音区的上下文承接关系。这会让多音区交互被误建模成多个彼此隔离的会话，导致车辆状态、工具调用和对话上下文无法自然共享。

## What Changes

- 在用户消息中携带音区和用户标识，默认用户为 `guest`。
- 保留一份所有音区共享的对话历史，让模型基于 System Prompt 和带音区标签的用户消息自行判断上下文承接关系。
- 扩展 System Prompt，明确多音区共享上下文、跨音区上下文判断、代发指令、个人隐私和冲突优先级规则。
- 在助手回复开头输出 `[zone=xxx]` 或 `[broadcast]` 前缀，由外层解析并路由到对应音区。
- 在工具执行层传入 `requesting_zone`，用于音区级权限校验和安全规则。
- 调整历史压缩逻辑，摘要中保留音区标签，避免压缩后丢失“谁发起了什么任务”。
- 扩展交互式 REPL，支持模拟音区输入，便于测试多音区对话。

## Capabilities

### New Capabilities
- `multi-zone-shared-context`: 用户消息携带音区与用户标签，所有音区共享同一条对话历史，模型自行判断跨音区上下文承接关系，并通过输出前缀指定播报音区。
- `zone-aware-tool-permissions`: 工具执行层基于发起音区执行权限和安全校验，避免跨音区代发绕过主驾专属或后排受限能力。

### Modified Capabilities
- `cli-interactive-chat`: REPL 需要支持选择或切换模拟音区，并在输入和输出中呈现音区上下文。

## Impact

- `config.py` 的 `SYSTEM_PROMPT_TEMPLATE` 需增加多音区契约。
- `cockpit_agent.py` 的 `chat_stream()` / `chat()` 需接收 `zone_id` 和 `user_id`，并在写入消息历史时拼入标签。
- `tool_gateway.py` 的 `execute()` 需接收 `requesting_zone`，并在安全校验中使用。
- 历史压缩逻辑需保留音区归属信息。
- `cli_repl.py` 需支持模拟音区和输出前缀解析。
- 不引入新的外部依赖；兼容 OpenAI-compatible chat message 格式，不依赖消息对象的 `name` 字段。
