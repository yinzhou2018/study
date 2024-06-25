## win 平台 CEF 离屏渲染示例

完整的网页离屏渲染功能需要做比较多的工作，主要两方面：

- 内容呈现：重点要注意处理 Popup 类型元素（比如`select`标签），鼠标变形及~~Native Tips~~呈现逻辑；
- 用户交互：输入事件，~~拖拽事件~~，输入法，~~可访问性~~等；

当前如上有删除线的功能没有实现，输入法处理相对复杂，重点解决了搜狗输入法的光标位置跟随问题。

### 性能问题

采用如下[跑马灯网页](https://33tool.com/marquee/)测试几点发现：

- CEF 事先创建了一个共享纹理池，每次网页渲染，从池子里获取一个可用共享纹理作为 gpu 渲染目标，如果应用打开`shared_texture_enabled`设置，则渲染结束直接将此共享纹理跨进程传递到应用上屏，否则创建一个设备无关位图(DIB)，将纹理拷贝填充此位图，然后将位图数据传递给应用呈现上屏；
- 基于上面的分析，共享纹理理论上是最高效的传送方式，实测不明显，两种方式性能数据相当，且时不时出现花屏现象，基于位图传递没有此问题；
- 保持`windowless_frame_rate`默认设置`30`，与`60`差异不大，应该大部分动画帧率 `30` 就足够；
- 会有固定时间段帧率下降到不到 `15`，会有顿挫感，即使把应用层的渲染去掉也一样，初步看起来 cef 内部离屏渲染实现可能有性能问题；
- 直接渲染到窗口与采用 DirectComposition 在窗口上叠加一层混合渲染没有任何性能差异；

### 命令行参数
- `--url=xxxxx`: 支持传入URL地址;
- `--single-thread-mode`: 启用单线程模式，默认开启独立CEF UI线程运行，避免窗口操作阻塞；
- `--frame-rate`：离屏渲染帧率，默认`30`,最大不能超过`60`;
- `--open-console`: 打开控制台实时输出渲染性能数据，默认不打开;
- `--shared-texture-disabled`: 禁用共享纹理方式传递数据，默认启用;
- `--composition-disabled`: 禁用基于[DirectComposition](https://learn.microsoft.com/en-us/windows/win32/directcomp/directcomposition-portal)合成渲染，默认启用;
- `--paint-disabled`: 禁用渲染上屏，只接收离屏数据，用于性能调试；
- `--log-render-cost-threshold`：渲染上屏耗时输出性能日志的阈值，单位:`ms`，默认值：`50`，用于性能调试;
- `--log-render-interval-threshold`: 两次离屏数据回调间隔输出性能日志的阈值，单位：`ms`，默认值：`50`，用于性能调试；

