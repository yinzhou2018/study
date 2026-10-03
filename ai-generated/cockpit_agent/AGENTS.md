# AGENTS.md

本文件用于指导 AI 编码助手（Agent）在本仓库中高效、正确地工作。开始改动前请通读全文，每次有新的代码实现后都要检查并同步变更本文档，保证始终反应当前工程的真实现状。

## 1. 项目简介

`cockpit_agent` 是一个**智能座舱车载助手**原型，使用 OpenAI 兼容接口的大模型，通过"工具集动态加载"机制为车内多音区、多用户提供车辆控制、信息查询、娱乐导航等对话服务。

核心设计目标：

- **固定 System Prompt**：最大化服务端 Prefix KV Cache 命中率，降低首字延迟。
- **工具集按需加载**：全量工具集过大，模型按用户意图调用 `load_toolsets` 激活子集，最多同时激活 3 个，LRU 淘汰。
- **多音区多用户**：front_left / front_right / rear_left / rear_right 共享同一辆车状态与对话上下文，按音区做权限隔离。
- **安全校验**：行车安全限制（如高速下车窗开度）、主驾专属权限（门锁、放电、泊车等）在网关层强制拦截。
- **多供应商 + 思考深度（effort）**：运行期可切换供应商与 effort（none/low/high/max），各供应商映射为各自的 payload 字段。

## 2. 技术栈

- 语言：Python 3.12（无框架，纯标准库 + `requests`）
- 依赖：见 `requirements.txt`，仅 `requests>=2.31.0`
- 运行环境：macOS / Linux（CLI 使用 `termios`+`tty`，Windows 回退 `msvcrt`）
- 虚拟环境：项目根 `venv/`（已 gitignore），VSCode 默认环境管理器为 venv

## 3. 目录结构

```
cockpit_agent/
├── main.py                # 入口：argparse 解析 --effort/--provider/--interactive，跑演示多轮对话
├── cockpit_agent.py       # CockpitAgent：核心对话编排（chat / chat_stream / compress_history）
├── llm_client.py          # MockLLMClient（离线跑通）+ OpenAICompatibleLLM（真实接口/流式）
├── llm_config.py          # 供应商注册表 + effort→payload 映射（⚠️ 已 gitignore，含真实密钥）
├── config.py              # System Prompt 模板、系统工具定义、音区/车辆状态常量
├── toolset_manager.py     # ToolsetManager：加载/LRU淘汰/动态组装工具列表/生成 toolset listing
├── tool_gateway.py        # ToolGateway：白名单→权限→安全→执行，集中拦截
├── zone_context.py        # 多音区用户消息标签的打包/解析（[zone=xx,user=yy]）
├── cli_repl.py            # CockpitRePL：交互式 REPL + Esc 打断监听 + 耗时统计
├── prefs.py               # provider/effort 偏好持久化：~/.cockpit_agent/prefs.json 读写与启动优先级解析
├── toolsets.json          # 全量工具集定义（12 个工具集，工具 ID 与 enum 的唯一真相源）
├── toolsets.md            # 工具集人类可读文档（由 toolsets.json 派生，仅供参考）
├── test_*.py              # 单元测试（unittest）
├── requirements.txt
├── .gitignore             # 忽略 venv/ __pycache__/ llm_config.py
└── .vscode/settings.json
```

## 4. 架构与数据流

```
用户输入
  │  zone_context.tag_user_message  →  [zone=front_left,user=guest] 有点闷
  ▼
CockpitAgent.chat / chat_stream
  │  ┌─ messages 历史（System Prompt 在前，全程不变）
  │  ├─ ToolsetManager.get_current_tools()  →  系统工具 + 已激活业务工具
  │  └─ LLMClient.chat / chat_stream (effort, tools)
  ▼
模型返回 tool_calls？
  ├─ load_toolsets → ToolsetManager.load_toolsets（LRU 淘汰，更新可用工具）
  ├─ list_active_toolsets / 业务工具 → ToolGateway.execute
  │        └─ check_whitelist → check_permission(音区) → check_security(车速等) → 执行 → 回填 tool 结果
  └─ 无 tool_calls → 最终回复，结束
```

- 单轮用户输入最多 `max_turns=8` 轮工具调用，防死循环。
- 流式模式支持 Esc 打断（`agent.interrupt()` 置标志，循环中 `_check_interrupt()` 抛 `InterruptedError`）。
- `compress_history()`：超过 6 条消息时，把中间 tool 结果压缩成"执行摘要" system 消息，保留 System Prompt + 最近 4 条。

## 5. 关键模块职责

| 模块                 | 职责                                                                                                                                                               | 改动注意                                                            |
| -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------- |
| `config.py`          | System Prompt 模板、`build_system_prompt`、`build_system_tools`（系统工具的 enum 动态来自 toolsets.json）、音区常量、`MOCK_VEHICLE_STATE`、`MAX_ACTIVE_TOOLSETS=3` | System Prompt 模板**全程固定**，改动会破坏 Prefix Cache 假设        |
| `toolset_manager.py` | 从 `toolsets.json` 加载全量配置；`load_toolsets` 实现 LRU 淘汰；`get_current_tools` 动态拼装；`get_toolset_listing` 生成 System Prompt 中的工具集清单              | 工具集 ID 是 enum 真相源，新增/删除工具集必须同步 `toolsets.json`   |
| `tool_gateway.py`    | 三段校验（白名单→权限→安全）后执行；`DRIVER_ONLY_TOOLS` 定义主驾专属工具                                                                                           | 新增涉及行车安全/门锁/放电/泊车的工具，务必加入 `DRIVER_ONLY_TOOLS` |
| `zone_context.py`    | 用户消息打 `[zone=,user=]` 前缀，解析用户消息音区标签                                                                                                            | 多音区规则见 System Prompt「多音区多用户对话」段                    |
| `llm_client.py`      | `MockLLMClient` 关键词匹配模拟工具调用链；`OpenAICompatibleLLM` 支持运行期 `configure` 重配供应商                                                                  | 流式需正确合并 `tool_calls` 增量（按 index 累加 name/arguments）    |
| `llm_config.py`      | `PROVIDERS` 注册表，每供应商绑定 `build_effort_payload`；`build_effort_payload(effort, provider)` 统一入口                                                         | **已 gitignore**，含真实 api_key；改动需本地保留，勿提交            |
| `cockpit_agent.py`   | 编排：消息管理、工具调用分派、打断、历史压缩、`set_provider` 运行期切换                                                                                            | `load_toolsets` 走 manager，其余工具走 gateway                      |
| `cli_repl.py`        | REPL 命令 `/exit /clear /history /effort /provider /zone`；`EscListener` 后台线程监听 Esc                                                                          | 打断后需 `_flush_input` 清空残留输入，否则污染下一次 `input()`      |
| `prefs.py`           | 偏好持久化（`~/.cockpit_agent/prefs.json`）：`load_prefs`/`save_prefs`/`update_pref`/`resolve_pref`                                                              | 不依赖业务配置，校验由调用方传入 valid 集合，避免循环依赖             |

## 6. 运行方式

```bash
# 激活虚拟环境（若未创建：python3 -m venv venv）
source venv/bin/activate
pip install -r requirements.txt

# 演示多轮对话（使用 MockLLMClient，无需真实密钥）
python3 main.py

# 指定供应商与思考深度
python3 main.py --provider voyah --effort high

# 交互式 REPL（流式输出，Esc 打断，支持 /effort /provider /zone 切换）
python3 main.py -i
```

> 真实供应商调用依赖本地 `llm_config.py`（已 gitignore）。未提供时，MockLLMClient 可跑通全部流程。
> **偏好持久化**：REPL 中 `/effort` `/provider` 切换会写入 `~/.cockpit_agent/prefs.json`，下次启动自动恢复；命令行显式参数优先且不写回。优先级：命令行 > 持久化 > 代码默认。

## 7. 测试约定

- 框架：`unittest`，测试文件 `test_*.py`。
- 运行：`python3 -m unittest discover -p 'test_*.py'`
- 测试通过 `monkeypatch llm_client.requests.post` 或 `FakeLLMClient` 注入，**不发起真实网络请求**。

## 8. 编码规范

- 缩进 2 空格；中文字符串与注释使用简体中文。
- 函数/方法体内流程用空行分段，注释简洁说明意图。
- 面向"配置驱动"：工具集、供应商、effort 均以注册表/JSON 形式声明，避免散落硬编码。
- 类型注解按需添加（如 `zone_context.py` 使用 `tuple[str | None, ...]`），不强求全覆盖。
- 一个函数只做一件事，关注一个点，有效代码行建议 ≤20行，最大禁止超过40行
- 一个类只做一件事，关注一个点，成员变量禁止超过7个
- 需求不清楚时跟人类澄清，不要自作聪明
- 除非非常简单的局部修改，否则必须先有设计并跟人类确认（每次提出了修改意见都必须重新发起确认确保理解正确）再做实际编码
- 除非非常简单的局部修改，否则必须写测试代码并保证新增用例以及受影响的用例全部测试通过

## 9. 安全与合规要点

- **密钥不入库**：`llm_config.py` 已 gitignore，禁止把真实 `api_key`/`base_url` 写进其它已跟踪文件或提交。
- **权限边界**：非主驾音区禁止调用 `DRIVER_ONLY_TOOLS`（门锁、儿童锁、尾门、泊车、充电、V2L 放电等），网关层强制拦截。
- **行车安全**：高速（>60km/h）下车窗开度限制在 25%，新增动态控制工具需在 `ToolGateway.check_security` 增加对应规则。
- **隐私隔离**：通讯/消息/日程等涉及个人隐私的工具，回复仅对发起音区，不跨区泄露。

## 10. 常见任务指引

- **新增工具集**：在 `toolsets.json` 添加条目（含 `name`/`description`/`tools`/可选 `execution_rules`）→ 在 `tool_gateway.py` 实现各工具执行分支 → 涉及安全/权限的补充 `check_security`/`DRIVER_ONLY_TOOLS` → 同步更新 `toolsets.md` 文档。
- **新增供应商**：在 `llm_config.py` 的 `PROVIDERS` 注册，绑定该供应商的 `build_effort_payload` 函数 → 确保 `build_effort_payload(effort, provider_id)` 覆盖所有 effort 模式。
- **调整思考深度**：`EFFORT_MODES`/`DEFAULT_EFFORT` 在 `llm_config.py`；改动映射函数后同步修测试。
- **持久化偏好**：`prefs.py` 读写 `~/.cockpit_agent/prefs.json`；`resolve_pref` 负责命令行>持久化>默认的优先级。新增需持久化的偏好项时，在 `main.py` 启动解析与 `cli_repl.py` 切换处同步接入，并 monkeypatch `PREFS_FILE` 补测试。
- **调整 System Prompt**：改 `config.py` 的 `SYSTEM_PROMPT_TEMPLATE`，注意这是"全程固定"的缓存前缀，频繁改动会削弱缓存收益。

## 11. 提交检查清单

- [ ] `python3 -m unittest discover -p 'test_*.py'` 通过（或已记录预期失败原因）
- [ ] 未提交 `llm_config.py`、`venv/`、`__pycache__/`
- [ ] 未在已跟踪文件中硬编码真实密钥
- [ ] 新增工具集/供应商/effort 字段时，`toolsets.md`、测试、文档同步更新
