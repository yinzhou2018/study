## Context

当前 cockpit_agent 以批处理演示模式运行：`main.py` 硬编码一组查询，依次调用 `CockpitAgent.chat()`，该方法内部同步阻塞调用 LLM，完成任务后一次性打印结果。LLM 客户端（`OpenAICompatibleLLM`）使用 `requests.post` 发送非流式请求，`MockLLMClient` 按预设逻辑返回完整响应。用户无法在生成过程中观察进度或打断操作。

现有模块结构：
- `llm_client.py`: `MockLLMClient` / `OpenAICompatibleLLM`，均实现 `chat()` 同步方法
- `cockpit_agent.py`: `CockpitAgent.chat()` 在工具调用循环中逐轮调用 `self.llm.chat()`
- `tool_gateway.py` / `toolset_manager.py`: 工具执行与动态工具集管理，不涉及流式逻辑
- `config.py`: System Prompt 模板与车辆模拟状态

## Goals / Non-Goals

**Goals:**
- 提供命令行交互式 REPL，用户可连续多轮对话
- LLM 响应流式输出：逐 token 显示思考内容与最终回复
- 工具调用过程实时展示：工具名、参数、执行状态、返回结果
- 支持 Esc 键打断 LLM 生成或工具执行，打断后安全回到提示符
- 会话命令：`/exit`、`/clear`、`/history`
- 保留现有批量演示模式，不破坏现有 API

**Non-Goals:**
- 不实现 Web UI 或 GUI 界面
- 不修改工具集管理、网关安全校验逻辑
- 不实现多用户会话或持久化存储
- 不实现语音输入/输出（TTS/STT）

## Decisions

### 1. 流式 LLM 客户端：基于 SSE 的 `chat_stream` 方法

`OpenAICompatibleLLM` 新增 `chat_stream(messages, tools, temperature)` 方法，使用 `requests.post(..., stream=True)` 逐行读取 SSE `data:` 行，解析 delta content 和 tool_calls 增量。选择 SSE 而非 WebSocket，因为 OpenAI 兼容 API 标准即 SSE，无需额外协议。

备选方案：使用 `httpx` 或 `aiohttp` 异步流式。排除原因：引入异步会增加整个调用链复杂度（toolset_manager、tool_gateway 均为同步），收益不明确；同步 `requests` 流式已能满足逐 token 输出加 Esc 打断需求。

`MockLLMClient` 新增 `chat_stream` 方法，用 `time.sleep` 模拟逐 token yield，便于无网络环境测试流式渲染。

### 2. CockpitAgent 流式重构：回调驱动而非返回值

`CockpitAgent` 新增 `chat_stream(user_query, on_token, on_tool_call, on_tool_result, on_done)` 方法。在工具调用循环中：
- 逐 token 调用 `on_token(text)` 将 LLM 输出推给前端
- 检测到 tool_calls 时调用 `on_tool_call(tool_name, arguments)`
- 工具执行后调用 `on_tool_result(tool_name, result)`

工具调用循环本身不变（load_toolsets 特殊处理仍保留），只是输出方式从 `print` 改为回调。

保留原 `chat()` 方法不变，内部委托给 `chat_stream` 并收集结果，确保向后兼容。

### 3. 打断机制：Esc 键监听 + 中断标志

REPL 在生成期间进入 raw 模式监听按键，检测到 Esc 键（0x1B）时设置 `self._interrupted = True`。`chat_stream` 在每次 yield token / 每轮工具调用前检查该标志，若为 True 则抛出 `InterruptedError` 或直接 break 回到调用方。打断后：
- 已完成的 tool_calls 结果保留在 messages 中
- 未完成的 LLM 生成丢弃部分 content，已收到的 content 保留
- 打印 `\n[已打断]` 后回到提示符

选择 Esc 而非 Ctrl+C 的原因：Ctrl+C 在 Python 中直接触发 KeyboardInterrupt，在 `input()` 阻塞时难以优雅捕获且容易导致整个进程退出；Esc 键语义更清晰（仅打断当前生成，不退出程序），且可通过 `tty.setraw()` + `select` 非阻塞单字符读取实现，不干扰信号处理链。

备选方案：使用 SIGINT（Ctrl+C）。排除原因：KeyboardInterrupt 在 `input()` 阻塞期间行为不可控，容易导致进程意外退出而非回到提示符。

### 4. REPL 模块设计：`cli_repl.py` 独立模块

新增 `cli_repl.py` 封装交互逻辑，包含：
- `CockpitRePL` 类：管理输入循环、命令解析、流式渲染
- 输入解析：以 `/` 开头为会话命令，否则作为用户消息
- 流式渲染：token 实时写入 stdout（无缓冲），工具调用以格式化块展示
- 退出命令 `/exit`，清空历史 `/clear`，查看历史 `/history`

`main.py` 增加 `--interactive` / `-i` 命令行参数，选择启动 REPL 还是批量演示。

### 5. 流式输出格式

```
用户: 有点闷

[思考中] 正在分析你的请求...
[工具调用] load_toolsets({"toolset_ids": ["toolset_quick_ventilation"]})
[工具结果] 成功加载1个，失败0个
[工具调用] ctrl_window_left_front({"openness": 25})
[工具结果] 左前车窗已调至25%
[最终回复] 好的，已为你打开左前车窗25%...
```

思考过程和最终回复均逐 token 流式输出，工具调用以格式化块展示。

## Risks / Trade-offs

- [SSE 解析兼容性] 不同 OpenAI 兼容服务（vLLM/Ollama/云端）SSE 格式可能略有差异，解析器宽松处理，兼容 `data: [DONE]` 和不同 chunk 结构
- [打断后消息一致性] 打断时 messages 中可能残留不完整的 assistant 消息，打断时检查最后一条消息，若 content 不完整则标记截断，下轮对话时 LLM 可理解上下文
- [MockLLM 流式模拟] 模拟客户端无法完全还原真实流式行为，Mock 逻辑尽量贴近真实 SSE delta 格式，测试覆盖流式解析路径
- [Esc 键跨平台兼容] Windows 下 `tty.setraw()` 行为不同，需用 `msvcrt` 模块做 Windows 兜底；macOS/Linux 上 `termios` + `tty` 标准库即可
- [Raw 模式与 input() 冲突] 监听 Esc 需要终端 raw 模式，而 `input()` 默认用 cooked 模式，使用 `select` + `sys.stdin` 非阻塞读取在生成期间检测 Esc，空闲时回到 `input()` 接受用户输入

## Migration Plan

1. 新增 `cli_repl.py` 和流式方法，不修改现有方法签名
2. `main.py` 增加 `--interactive` 参数，默认仍为批量演示模式
3. 部署后用户可通过 `python main.py --interactive` 进入交互模式
4. 如遇问题，去掉 `--interactive` 参数即可回退到原有模式

## Open Questions

- 流式输出是否需要颜色/样式（如 ANSI 颜色区分思考/工具/回复）？建议初始版本用纯文本前缀区分，后续可加颜色。
- 是否需要保存对话历史到文件？当前 Non-Goal，后续可扩展。
- 支持 Esc 键打断 LLM 生成或工具执行，打断后安全回到提示符
