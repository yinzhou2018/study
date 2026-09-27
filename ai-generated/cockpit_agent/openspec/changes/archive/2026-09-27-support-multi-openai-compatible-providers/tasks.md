## 1. Provider Profile Registry

- [x] 1.1 重构 `llm_config.py`：新增 `PROVIDERS` 字典与 `DEFAULT_PROVIDER_ID`，每个 profile 含 `base_url`、`api_key`、`model`、`build_effort_payload`
- [x] 1.2 保留 `LLM_BASE_URL`/`LLM_API_KEY`/`LLM_MODEL_ID` 作为向后兼容别名，指向默认 provider 对应字段
- [x] 1.3 内置至少两个示例 provider（当前 voyah 网关 + deepseek），分别使用 `reasoning_effort` 与 `enable_thinking` payload 映射

## 2. Effort Logic 迁移与 Strategy 注册表

- [x] 2.1 将 `EFFORT_MODES`、`DEFAULT_EFFORT`、`build_effort_payload` 从 `config.py` 迁移到 `llm_config.py`，并从 `config.py` 移除 effort 相关逻辑
- [x] 2.2 在 `llm_config.py` 为每个 provider 实现一个 `build_effort_payload` 函数，并直接放入 `PROVIDERS` profile
- [x] 2.3 将 `build_effort_payload` 签名改为 `build_effort_payload(effort, provider=None)`，从 provider profile 取对应函数分派，`provider=None` 回退到默认供应商
- [x] 2.4 无效 effort 仍抛 `ValueError`，未知 provider 回退到默认供应商
- [x] 2.5 更新 `llm_client.py`、`cli_repl.py`、`main.py`、`test_effort.py` 的 effort 相关 imports，统一改为从 `llm_config.py` 导入

## 3. OpenAICompatibleLLM 适配

- [x] 3.1 `__init__` 增加可选 `provider_id=None` 并存储属性
- [x] 3.2 `chat`/`chat_stream` 调用 `build_effort_payload(effort, self.provider_id)` 生成 payload

## 4. CockpitAgent Provider 支持

- [x] 4.1 `CockpitAgent.__init__` 增加 `provider_id` 参数，默认 `DEFAULT_PROVIDER_ID`，并存储 `self.provider_id`
- [x] 4.2 实现 `set_provider(provider_id)`：校验合法性、更新 `provider_id`、重配置底层 `OpenAICompatibleLLM`（或对 Mock 仅更新 id）
- [x] 4.3 切换 provider 时保留 `messages` 历史不变

## 5. CLI 接入

- [x] 5.1 `main.py` 新增 `--provider` 参数，校验有效性，启动时按 profile 构造 `OpenAICompatibleLLM` 并传入 `provider_id`
- [x] 5.2 `cli_repl.py` 新增 `/provider` 命令：无参打印当前 provider 与可选列表，有参切换并打印模型，无效打印可选列表
- [x] 5.3 更新 REPL 欢迎横幅与未知命令提示，包含 `/provider`

## 6. 测试

- [x] 6.1 扩展 `test_effort.py`：确认 effort imports 来自 `llm_config.py`，并新增各 provider 的 payload 映射测试
- [x] 6.2 新增 provider registry 测试：`PROVIDERS` 非空、profile 字段齐全、未知 id 抛错
- [x] 6.3 新增 `CockpitAgent.set_provider` 测试：合法切换更新 id 与客户端配置、非法抛错、历史保留
- [x] 6.4 新增 CLI `/provider` 命令测试（可选，若 REPL 测试基础设施允许）
- [x] 6.5 运行全量 `python -m pytest` / `python -m unittest` 确保现有测试通过
