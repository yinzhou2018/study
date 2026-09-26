## Context

当前系统的 LLM 调用链路为：CLI REPL → CockpitAgent → LLM Client（OpenAICompatibleLLM 或 MockLLMClient）。LLM 客户端的 `chat` 和 `chat_stream` 方法目前接收 `temperature` 参数，但没有任何控制推理深度的参数。DeepSeek API 等兼容接口支持 `reasoning_effort` 字段来控制思考深度，取值为 `none`/`low`/`high`/`max`（部分 API 使用 `minimal` 代替 `none`）。

REPL 已有斜杠命令机制（`/exit`、`/clear`、`/history`），扩展 `/effort` 命令在架构上是自然的增量。关键设计点在于 effort 状态在调用链中的传递方式，以及不同 API 对 `reasoning_effort` 参数的兼容性处理。

## Goals / Non-Goals

**Goals:**
- 用户可以在 REPL 中通过 `/effort` 命令实时切换推理深度
- effort 设置贯穿到 LLM 请求 payload，实际影响模型行为
- `none` 模式有效关闭推理，减少延迟
- 默认 `high` 模式确保复杂场景的推理质量

**Non-Goals:**
- 不支持基于上下文自动调节 effort（纯手动切换）
- 不在批处理模式（batch demo）中暴露 effort 控制
- 不持久化 effort 设置到磁盘（仅会话内有效）

## Decisions

**1. effort 状态存储在 CockpitAgent 上**

Agent 是调用链的中间层，REPL 持有 agent 引用。将 `self.effort` 放在 agent 上，REPL 通过 `self.agent.effort` 读写，agent 在调用 `self.llm.chat_stream()` 时传递该值。这比放在 REPL 或 LLM 客户端上更合理，因为 agent 是业务逻辑的编排者。

备选方案：放在 LLM 客户端上。不采用，因为客户端应该是无状态的工具，effort 是会话级业务状态。

**2. effort 参数通过方法签名传递，不通过全局变量**

`chat` 和 `chat_stream` 方法增加 `effort` 关键字参数，默认值为 `None`（表示不传 `reasoning_effort` 字段，由 API 用默认行为）。agent 调用时传入 `self.effort`。这样保持了客户端的无状态性，也兼容不关心 effort 的调用方（如 batch demo）。

备选方案：在客户端构造时传入 effort。不采用，因为 effort 需要运行时动态切换。

**3. effort 模式映射到 API 参数**

在 `config.py` 中定义 `EFFORT_MODES` 映射：
- `none` → 不在 payload 中设置 `reasoning_effort` 字段，同时设置 `reasoning=False`（DeepSeek API 支持）
- `low` → `reasoning_effort: "low"`
- `high` → `reasoning_effort: "high"`
- `max` → `reasoning_effort: "max"`

`none` 模式的处理：部分 API 用 `reasoning_effort: "none"`，部分用 `reasoning: false`。采用双保险策略：同时传 `reasoning_effort: "none"` 和 `reasoning: false`，由 API 自行忽略不认识的字段。

**4. REPL 命令解析**

`/effort` 不带参数 → 显示当前模式
`/effort <mode>` → 切换模式，校验合法性，非法值打印错误和可用列表
欢迎信息和帮助文本中新增 `/effort` 命令说明

**5. 启动参数 `--effort`**

`main.py` 的 argparse 增加 `--effort` 参数，覆盖默认值 `high`。这允许脚本化使用时预设 effort 模式。

## Risks / Trade-offs

- [API 兼容性] 不同 LLM 提供商对 `reasoning_effort` 的支持不一致 → 通过 `none` 模式的双保险策略缓解；不认识该字段的 API 会忽略它，不会报错
- [Mock 客户端行为] MockLLMClient 不真实调用 API → Mock 客户端接收 effort 参数但忽略其实际效果，仅保证接口兼容
- [会话内有效] effort 不持久化 → 用户每次启动 REPL 需重新设置。可通过 `--effort` 启动参数缓解
- [reasoning_content 流式显示] `none` 模式下不会有 reasoning 输出 → REPL 的 `on_reasoning` 回调不会被触发，现有逻辑无需修改即可兼容
