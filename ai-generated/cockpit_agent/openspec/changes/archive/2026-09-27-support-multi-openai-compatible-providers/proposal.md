## Why

当前 LLM 供应商在 `llm_config.py` 中硬编码为单一常量，`main.py` 启动时构造唯一 `OpenAICompatibleLLM` 实例，运行期间无法切换。`config.build_effort_payload` 以全局方式把 effort 映射成 `reasoning_effort` / `reasoning: false`，但不同 OpenAI 兼容供应商在表达"思考深度"时字段名、取值范围和禁用方式存在细微差别（如 `reasoning_effort`、`enable_thinking`+`thinking_budget`、`reasoning` 布尔、或不支持任何字段）。这导致切换供应商时 effort 控制可能失效或产生非法字段。

## What Changes

- 引入"供应商配置（provider profile）"概念，每个 profile 包含 `base_url`、`api_key`、`model` 和 `build_effort_payload` 函数，集中管理多个 OpenAI 兼容供应商。
- 将 effort 相关常量、模式定义、payload 映射和供应商级分派全部集中到 `llm_config.py`，`config.py` 不再承载 LLM effort 逻辑。
- 在运行期支持供应商切换：启动时通过 `--provider` 选择，交互模式通过 `/provider` 命令查看与切换。
- 将 `build_effort_payload` 由全局映射重构为按供应商分派：每个 provider profile 直接绑定自己的 `build_effort_payload` 函数，保留 `none/low/high/max` 四档语义不变，但具体 payload 字段随供应商变化。
- `CockpitAgent` 持有当前 provider id，并在每次 LLM 调用时把 provider 信息传入客户端，使同一 agent 实例可在运行期切换供应商。
- **BREAKING**：`build_effort_payload` 签名变为 `build_effort_payload(effort, provider=None)`，调用方需传入 provider profile 或 provider id；旧的只传 effort 的调用得到默认供应商映射的向后兼容行为。

## Capabilities

### New Capabilities
- `llm-provider-management`: 多 OpenAI 兼容供应商的注册、选择、运行期切换，以及供应商级 effort payload 分派。

### Modified Capabilities
- `effort-control`: effort 到请求 payload 的映射不再固定为 `reasoning_effort` / `reasoning: false`，而是由当前供应商绑定的 `build_effort_payload` 函数决定字段名与取值。
- `cli-interactive-chat`: 新增 `/provider` 会话命令用于查看与切换当前 LLM 供应商。

## Impact

- `llm_config.py`：从单一常量重构为 provider profile 表，并集中持有 `EFFORT_MODES`、`DEFAULT_EFFORT`、每个供应商的 `build_effort_payload` 函数与全局分派入口。
- `config.py`：移除 effort 常量与 payload 映射函数，仅保留非 LLM 配置。
- `llm_client.py`：`OpenAICompatibleLLM` 接受 `provider_id` 并据此选择 effort 映射；修正非流式 URL。
- `cockpit_agent.py`：持有 provider id，并在切换供应商时重配置客户端。
- `main.py`：新增 `--provider` 启动参数。
- `cli_repl.py`：新增 `/provider` 命令及帮助文本。
- 测试：`test_effort.py` 需扩展为多供应商 effort mapping；新增 provider 注册与切换测试。
