## ADDED Requirements

### Requirement: 消息模型定义

系统 SHALL 定义泛型化的流式消息模型 `StreamMessage<T>`，包含消息 ID、消息类型、消息体三个核心字段。

- 消息类型 SHALL 包括：`DATA`（数据）、`COMPLETE`（完成）、`ERROR`（错误）、`PING`（心跳）
- 消息体 SHALL 支持泛型类型 `T`，通过 JSON 序列化/反序列化
- 消息 ID SHALL 使用 UUID 字符串

#### Scenario: 构造 DATA 消息
- **WHEN** 创建一个类型为 `DATA` 的 `StreamMessage<String>`，消息体为 "hello"
- **THEN** 消息 ID 不为空，类型为 `DATA`，消息体为 "hello"；JSON 序列化后包含 `msgId`、`type=DATA`、`payload="hello"`

#### Scenario: 构造 COMPLETE 消息
- **WHEN** 创建一个类型为 `COMPLETE` 的 `StreamMessage<Void>`
- **THEN** JSON 序列化后 `type=COMPLETE`，payload 为空

### Requirement: 消息发送者接口

系统 SHALL 定义 `StreamSender<T>` 接口，用于向上行（发送方向）写入流式消息。

- `send(StreamMessage<T>)` — 发送一条消息
- `complete()` — 通知发送完成
- `cancel(Throwable cause)` — 取消发送并传递错误原因

#### Scenario: 发送多条消息并完成
- **WHEN** 调用 `sender.send(msg1)` 发送消息，然后调用 `sender.send(msg2)`，再调用 `sender.complete()`
- **THEN** 所有消息按顺序发送，`complete()` 后不再接收新消息

#### Scenario: 取消发送
- **WHEN** 调用 `cancel(new IOException("cancel"))`
- **THEN** 未发送的消息被丢弃，接收端收到 error 信号

### Requirement: 消息接收者接口

系统 SHALL 定义 `StreamReceiver<T>` 接口，用于接收下行流式消息并注册回调。

- `onMessage(Consumer<StreamMessage<T>>)` — 注册消息到达回调
- `onComplete(Runnable)` — 注册流完成回调
- `onError(Consumer<Throwable>)` — 注册错误回调

#### Scenario: 接收消息流
- **WHEN** 注册 `onMessage` 回调后，收到三条 DATA 消息，然后收到 COMPLETE
- **THEN** `onMessage` 被调用三次，之后 `onComplete` 被调用一次

#### Scenario: 流出错
- **WHEN** 流被取消，收到错误信号
- **THEN** `onError` 回调被调用，携带错误原因

### Requirement: 双向流调用接口

系统 SHALL 定义 `StreamCall<ReqT, RespT>` 接口，将上行请求和下行响应绑定为一个调用上下文。

- `requestStream()` — 返回 `StreamSender<ReqT>`
- `responseStream()` — 返回 `StreamReceiver<RespT>`
- `await()` — 阻塞等待调用完成

#### Scenario: 发起请求并接收响应
- **WHEN** 创建 `StreamCall<String, String>`，通过 `requestStream().send(request)` 发送请求，然后通过 `responseStream().onMessage(...)` 接收响应
- **THEN** 上行请求和下行响应在同一调用上下文中关联

### Requirement: 流式客户端接口

系统 SHALL 定义 `StreamClient` 接口，管理 QUIC 连接和流式调用。

- `connect(String host, int port)` — 连接到服务端
- `newCall(String path, Class<ReqT>, Class<RespT>)` — 创建新的流式调用
- `shutdown()` — 优雅关闭连接

#### Scenario: 创建连接并发起调用
- **WHEN** 调用 `connect("localhost", 9999)` 成功，然后 `newCall("/echo", String.class, String.class)` 创建调用
- **THEN** 返回的 `StreamCall<String, String>` 可用于收发消息

#### Scenario: 优雅关闭
- **WHEN** 调用 `shutdown()`
- **THEN** 所有活跃流被通知完成，UDP 连接关闭

### Requirement: 流式服务端接口

系统 SHALL 定义 `StreamServer` 接口，监听 QUIC 连接并分发流式调用。

- `registerService(String path, StreamHandler<ReqT, RespT>)` — 注册服务处理函数
- `start(int port)` — 启动服务端监听
- `stop()` — 停止服务端

#### Scenario: 注册服务并启动
- **WHEN** 注册 `/echo` 路径的处理函数，然后调用 `start(9999)`
- **THEN** 服务端在 UDP 9999 端口监听 QUIC 连接

#### Scenario: 停止服务
- **WHEN** 调用 `stop()`
- **THEN** 所有活跃连接被通知关闭，UDP 端口释放