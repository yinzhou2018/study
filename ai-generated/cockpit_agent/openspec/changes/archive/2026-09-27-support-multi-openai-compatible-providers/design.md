## Context

`llm_config.py` 目前暴露三个常量 `LLM_BASE_URL` / `LLM_API_KEY` / `LLM_MODEL_ID`，`main.py` 启动时用它们构造唯一 `OpenAICompatibleLLM`。`config.build_effort_payload` 是全局函数，固定把 `none` 映射为 `reasoning: false`、其余映射为 `reasoning_effort: <mode>`。`OpenAICompatibleLLM.chat` 请求 `/v1/chat/completions`，而 `chat_stream` 请求 `/chat/completions`（缺少 `/v1`），两者端点不一致。交互模式已有 `/effort`、`/zone` 命令，但没有供应商切换入口。

## Goals / Non-Goals

**Goals:**
- 以 provider profile 表达多个 OpenAI 兼容供应商，运行期可切换且无需重启。
- 保留 `none/low/high/max` 四档 effort 语义，但 payload 字段由供应商绑定的 `build_effort_payload` 函数决定。
- 修正 `chat` 与 `chat_stream` 的端点拼接，使其对所有供应商一致且可预测。
- 保持现有 `CockpitAgent`/`MockLLMClient` 调用链与测试风格，改动局部可控。

**Non-Goals:**
- 不做异步并发请求、多供应商负载均衡或自动 failover。
- 不引入网络层重试、熔断或限流。
- 不替换 `requests` 为 SDK；仍用现有同步 HTTP 调用风格。
- 不实现密钥的加密存储或远程配置中心；provider profile 仍为代码内常量。

## Decisions

### Provider Profile 数据结构
用普通 dict 表达 provider profile，字段：`base_url`、`api_key`、`model`、`build_effort_payload`。`llm_config.py` 维护 `PROVIDERS` 字典（key 为 provider id）和 `DEFAULT_PROVIDER_ID`。选择 dict 而非 dataclass，与现有 `MOCK_VEHICLE_STATE` 等配置风格一致，避免引入新抽象。

### Effort 相关逻辑集中到 llm_config
`llm_config.py` 统一持有 LLM 侧配置：provider profile、`EFFORT_MODES`、`DEFAULT_EFFORT`、每个供应商的 `build_effort_payload` 函数与全局分派入口。`config.py` 移除 effort 相关常量和函数，仅保留 system prompt、多音区、车辆状态等应用配置。这样"供应商怎么配"和"effort 怎么映射"属于同一个 LLM 配置边界，避免跨模块分散。

### 每个供应商直接绑定 effort payload 函数
不引入 `effort_strategy` 字符串间接层，而是在 provider profile 中直接存放 `build_effort_payload` 函数。`llm_config.py` 为每个供应商实现一个函数：
- voyah：`none` -> `{"reasoning": False}`，`low/high/max` -> `{"reasoning_effort": <mode>}`（保持当前行为，作为默认映射）。
- deepseek：`none` -> `{"enable_thinking": False}`，`low/high/max` -> `{"enable_thinking": True, "thinking_budget": 512/2048/4096}`。

全局 `build_effort_payload(effort, provider=None)` 从 provider profile 取对应函数执行；`provider=None` 时使用默认供应商的函数，未知供应商也回退到默认。相关调用方（`llm_client.py`、`cli_repl.py`、`main.py`）统一从 `llm_config.py` 导入 effort 常量与函数。

### Agent 持有并切换 Provider
`CockpitAgent` 新增 `self.provider_id`，初始化时接受 `provider_id` 或默认值。`set_provider(provider_id)` 校验合法性后更新 `self.provider_id`，并重新配置底层 `OpenAICompatibleLLM` 的 `base_url`/`api_key`/`model`/`provider_id`。若底层是 `MockLLMClient`，切换仅更新 `provider_id`，不影响 mock 行为。这样同一 agent 与消息历史在切换供应商后继续可用。

### OpenAICompatibleLLM 接受 provider_id
`OpenAICompatibleLLM.__init__` 增加可选 `provider_id=None`，存储为属性。`chat`/`chat_stream` 调用 `build_effort_payload(effort, self.provider_id)`。保留旧的三参数构造方式以兼容现有调用方；不新增 `provider` 位置参数以免侵入 `MockLLMClient` 签名。

### CLI 入口
`main.py` 新增 `--provider` 参数，值为 `PROVIDERS` 的 key，缺省为 `DEFAULT_PROVIDER_ID`。`cli_repl.py` 新增 `/provider` 命令：无参打印当前供应商与可选列表；有参切换并调用 `agent.set_provider`，失败打印可选列表。

## Risks / Trade-offs

- [不同供应商实际字段语义可能超出当前内置函数] -> 新增供应商时在 `llm_config.py` 实现其 `build_effort_payload` 函数并注册到 `PROVIDERS` 即可，无需改 agent/客户端。
- [`build_effort_payload` 签名变化为 BREAKING] -> `provider` 参数可选且 None 回退到旧行为，降低实际破坏面；但文档标记 BREAKING 以提醒调用方逐步迁移。
- [运行期切换供应商后模型能力差异可能导致历史上下文不兼容] -> 不在框架层做消息转换，由用户自行 `/clear`；切换命令提示当前模型。

## Migration Plan

1. 重构 `llm_config.py` 为 `PROVIDERS` 表，保留 `LLM_BASE_URL`/`LLM_API_KEY`/`LLM_MODEL_ID` 作为向后兼容别名（指向默认 provider）。
2. 将 `config.py` 中的 `EFFORT_MODES`、`DEFAULT_EFFORT`、`build_effort_payload` 迁移到 `llm_config.py`，并在 `llm_config.py` 中为每个供应商实现 `build_effort_payload` 函数；相关 imports 同步改为 `llm_config`。
3. `CockpitAgent` 增加 `provider_id` 与 `set_provider`。
4. `main.py`/`cli_repl.py` 接入 `--provider` 与 `/provider`。
5. 扩展 `test_effort.py` 并新增 provider 切换测试。

## Open Questions

- 是否需要环境变量覆盖 provider profile 中的 api_key？当前建议后续按需加，不在此变更内。
- 是否需要在 `/provider` 切换时自动 `/clear` 历史？当前建议不自动清空，仅提示用户可手动清空。
