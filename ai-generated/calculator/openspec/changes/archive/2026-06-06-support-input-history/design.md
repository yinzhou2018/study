## Context

当前计算器 REPL 使用 `std::getline` 从 `std::cin` 读取输入，不支持行编辑和历史导航。交互模式下用户只能逐字符输入，无法使用方向键浏览或编辑已输入的行。非交互模式（管道输入）通过 `isatty()` 检测并保持简单 `std::getline` 读取。

## Goals / Non-Goals

**Goals:**
- 交互模式下支持上下方向键浏览输入历史
- 交互模式下支持基本行编辑（退格、删除、光标移动）
- 历史记录跨会话持久化至用户目录
- 非交互模式行为不变

**Non-Goals:**
- 不实现 Tab 补全功能
- 不实现历史搜索（Ctrl+R）
- 不实现多行输入编辑
- 不自定义历史记录存储格式（使用 linenoise 默认格式）

## Decisions

### 1. 使用 linenoise 作为行编辑库

**选择**: linenoise（antirez/linenoise）
**替代方案**:
- GNU readline：功能更全但 GPL 许可证受限，依赖较重
- editline/libedit：BSD 许可，但 API 较为复杂
- 自行实现：工作量大，需处理终端转义序列、平台兼容性

**理由**: linenoise 是单头文件 + 单源文件的轻量实现，BSD 许可证，支持历史持久化，足够满足当前需求。通过 CMake FetchContent 引入，无需系统级安装。

### 2. 历史持久化路径

**选择**: `~/.calculator_history`
**替代方案**:
- XDG 标准路径 `~/.local/share/calculator/history`
- 当前目录下 `.history`

**理由**: 单文件路径简单直接，与 linenoise 的 `linenoiseHistorySave`/`linenoiseHistoryLoad` API 天然匹配。未来如需遵循 XDG 标准可轻松迁移。

### 3. 输入读取策略

**选择**: 交互模式使用 linenoise，非交互模式保留 `std::getline`
**理由**: linenoise 需要终端环境，管道/重定向输入时不可用。通过已有的 `interactive_` 标志区分两种模式。

### 4. Repl 类改造方式

**选择**: 提取输入接口为抽象层，Repl 通过接口读取输入
**替代方案**:
- 直接在 Repl::Run() 中 if-else 分支调用 linenoise/getline

**理由**: 遵循单一职责和关注点分离原则，输入方式的差异封装在独立类中，Repl 无需感知底层实现。便于测试时注入模拟输入。

## Risks / Trade-offs

- [linenoise 在 Windows 上功能受限] → 当前项目仅针对 macOS/Linux，暂不处理 Windows 兼容性
- [历史文件无加密] → 计算器输入无敏感数据，风险可接受
- [FetchContent 在首次构建时需联网下载 linenoise] → 构建文档中说明首次构建需联网
