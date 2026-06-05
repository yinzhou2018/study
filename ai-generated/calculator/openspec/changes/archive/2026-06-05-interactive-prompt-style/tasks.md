## 1. 添加终端交互检测

- [x] 1.1 在`repl.h`中添加`bool interactive_`成员变量，并在构造函数中通过`isatty(fileno(stdin))`初始化
- [x] 1.2 在`repl.cc`中添加`<unistd.h>`头文件包含

## 2. 实现输入提示符

- [x] 2.1 在`Repl::Run()`的`while`循环中，`std::getline`调用前根据`interactive_`标志输出 "> " 提示符

## 3. 移除输出前缀

- [x] 3.1 在`Repl::ProcessLine()`中将`out_.get() << "= " << result.value()`修改为`out_.get() << result.value()`

## 4. 更新测试

- [x] 4.1 更新`test/repl_test.cc`中所有断言，将"= "前缀相关的期望值移除
- [x] 4.2 更新`test/integration_test.cc`中所有断言，将"= "前缀相关的期望值移除
- [x] 4.3 运行全部测试确认通过
