## 1. 构建配置

- [x] 1.1 在根 CMakeLists.txt 中通过 FetchContent 添加 linenoise 依赖
- [x] 1.2 在 src/CMakeLists.txt 中将 linenoise 链接至 calculator 目标

## 2. 输入接口抽象

- [x] 2.1 创建 `InputReader` 抽象基类（`src/input_reader.h`），定义 `ReadLine()` 纯虚函数和 `virtual` 析构函数
- [x] 2.2 创建 `StdinReader` 类（`src/input_reader.h` / `src/input_reader.cc`），封装 `std::getline` 读取逻辑，用于非交互模式
- [x] 2.3 创建 `LinenoiseReader` 类（`src/input_reader.h` / `src/input_reader.cc`），封装 linenoise 读取、历史加载和保存逻辑

## 3. Repl 改造

- [x] 3.1 修改 `Repl` 类，将输入流引用替换为 `std::unique_ptr<InputReader>` 成员
- [x] 3.2 修改 `Repl::Run()` 使用 `InputReader::ReadLine()` 替代 `std::getline`
- [x] 3.3 修改 `Repl` 构造函数，根据 `interactive_` 标志创建对应的 `InputReader` 实现
- [x] 3.4 在 `LinenoiseReader` 中设置 linenoise 提示符为 "> "，配置历史文件路径为 `~/.calculator_history`

## 4. 测试

- [x] 4.1 为 `StdinReader` 编写单元测试
- [x] 4.2 为 `LinenoiseReader` 编写单元测试（历史加载/保存）
- [x] 4.3 更新 `repl_test.cc` 适配新的 `InputReader` 接口
- [x] 4.4 运行全部测试确认通过

## 5. 清理

- [x] 5.1 运行 clang-format 格式化所有新增和修改的文件
- [x] 5.2 确认非交互模式（管道输入）行为与改造前一致
