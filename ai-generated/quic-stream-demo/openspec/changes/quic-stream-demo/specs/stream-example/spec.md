## ADDED Requirements

### Requirement: Echo 示例

系统 SHALL 提供 Echo 回声示例，演示最基本的请求-响应流式通信。

- 客户端发送一条或多条 DATA 消息
- 服务端原样返回每条消息（EchoHandler）
- 服务端收到 COMPLETE 后关闭流

#### Scenario: Echo 完整流程
- **WHEN** 启动 EchoServer，客户端连接并发送 "Hello, QUIC!"，然后发送 COMPLETE
- **THEN** 客户端收到 "Hello, QUIC!"，然后收到 COMPLETE

### Requirement: Chat 示例

系统 SHALL 提供 Chat 双向聊天示例，演示双方独立收发消息的能力。

- 客户端和服务端均可在同一流上独立发送消息
- 消息类型可区分（`JOIN`、`MESSAGE`、`LEAVE`）
- 通过 `StreamCall` 的 `requestStream()` 和 `responseStream()` 实现双向通信

#### Scenario: Chat 完整流程
- **WHEN** 客户端连接 Chat 服务，发送 JOIN 消息，然后发送几条 MESSAGE 消息
- **THEN** 服务端广播 JOIN 通知，对每条 MESSAGE 回复 ACK 或回复消息

### Requirement: StreamData 示例

系统 SHALL 提供 StreamData 服务端推送示例，演示服务端持续推送数据的能力。

- 客户端请求后，服务端持续推送多条消息（如时间戳、计数器、股票价格等）
- 服务端以固定间隔发送消息，客户端被动接收
- 客户端可发送 COMPLETE 取消推送

#### Scenario: 服务端推送数据流
- **WHEN** 客户端请求数据流，服务端每 1 秒推送一条消息
- **THEN** 客户端连续收到 5 条消息，然后发送 COMPLETE 停止推送

#### Scenario: 客户端取消推送
- **WHEN** 客户端收到 3 条消息后发送 COMPLETE
- **THEN** 服务端停止推送，流正常关闭