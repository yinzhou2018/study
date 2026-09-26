## 1. Config 层 — effort 模式定义

- [x] 1.1 在 `config.py` 中定义 `EFFORT_MODES` 常量，包含 `none`、`low`、`high`、`max` 四种模式
- [x] 1.2 定义 `DEFAULT_EFFORT` 常量值为 `"high"`
- [x] 1.3 定义 `build_effort_payload(effort)` 函数，将 effort 模式映射为 API payload 字段（`none` → `{"reasoning": False}`，`low`/`high`/`max` → `{"reasoning_effort": "<mode>"}`）

## 2. LLM 客户端层 — 接收并应用 effort

- [x] 2.1 在 `OpenAICompatibleLLM.chat()` 方法签名中增加 `effort=None` 关键字参数
- [x] 2.2 在 `OpenAICompatibleLLM.chat()` 的 payload 构建中调用 `build_effort_payload(effort)` 并合并字段
- [x] 2.3 在 `OpenAICompatibleLLM.chat_stream()` 方法签名中增加 `effort=None` 关键字参数
- [x] 2.4 在 `OpenAICompatibleLLM.chat_stream()` 的 payload 构建中调用 `build_effort_payload(effort)` 并合并字段
- [x] 2.5 在 `MockLLMClient.chat()` 方法签名中增加 `effort=None` 关键字参数（忽略实际效果，仅保持接口兼容）
- [x] 2.6 在 `MockLLMClient.chat_stream()` 方法签名中增加 `effort=None` 关键字参数（忽略实际效果）

## 3. Agent 层 — 持有并传递 effort 状态

- [x] 3.1 在 `CockpitAgent.__init__()` 中增加 `self.effort = DEFAULT_EFFORT` 属性
- [x] 3.2 在 `CockpitAgent.chat()` 中调用 `self.llm.chat()` 时传入 `effort=self.effort`
- [x] 3.3 在 `CockpitAgent.chat_stream()` 中调用 `self.llm.chat_stream()` 时传入 `effort=self.effort`

## 4. REPL 层 — `/effort` 命令

- [x] 4.1 在 `CockpitRePL._handle_command()` 中增加 `/effort` 命令解析逻辑
- [x] 4.2 实现 `/effort` 不带参数时显示当前模式
- [x] 4.3 实现 `/effort <mode>` 切换模式，校验合法性，非法值打印错误和可用列表
- [x] 4.4 更新欢迎信息和帮助文本，加入 `/effort` 命令说明
- [x] 4.5 更新未知命令的错误提示，加入 `/effort`

## 5. 启动参数 — `--effort` 命令行选项

- [x] 5.1 在 `main.py` 的 argparse 中增加 `--effort` 参数，choices 为 `none`/`low`/`high`/`max`，默认 `high`
- [x] 5.2 在 `main.py` 中将 `args.effort` 设置到 agent 的 effort 属性
- [x] 5.3 处理非法 effort 值的错误提示和退出逻辑

## 6. 测试验证

- [x] 6.1 编写单元测试：`build_effort_payload()` 对四种模式的输出正确性
- [x] 6.2 编写单元测试：`OpenAICompatibleLLM.chat()` payload 中包含正确的 effort 字段
- [x] 6.3 编写单元测试：`MockLLMClient` 接受 effort 参数不报错
- [x] 6.4 编写单元测试：`CockpitAgent` 的 effort 属性默认值和传递正确性
- [x] 6.5 手动验证 REPL 中 `/effort` 命令的查看、切换、非法值处理
- [x] 6.6 手动验证 `python main.py -i --effort low` 启动后 effort 为 low
- [x] 6.7 运行全部已有测试确保无回归
