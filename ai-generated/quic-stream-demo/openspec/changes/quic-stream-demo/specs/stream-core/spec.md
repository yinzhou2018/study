## ADDED Requirements

### Requirement: Netty QUIC 集成

系统 SHALL 基于 Netty + `netty-incubator-codec-quic` 实现 QUIC 客户端和服务端的底层通信。

- SHALL 使用 `QuicChannel` 和 `QuicStreamChannel` 作为 QUIC 连接和流的抽象
- SHALL 支持 QUIC v1 版本
- SHALL 使用自签名 TLS 证书进行握手（demo 模式）
- SHALL 支持并发多路复用（多个 QUIC stream 共享同一连接）

#### Scenario: 建立 QUIC 连接
- **WHEN** 客户端通过 `QuicChannel.bootstrap()` 向服务端发起 QUIC 连接
- **THEN** 连接建立成功，TLS 1.3 握手完成，返回 `QuicChannel` 实例

#### Scenario: 并发多路复用
- **WHEN** 客户端在同一连接上创建 3 个并发 QUIC stream
- **THEN** 3 个 stream 独立收发，互不阻塞

### Requirement: 消息编解码器

系统 SHALL 实现 `StreamMessageCodec` 编解码器，负责 `StreamMessage` 与字节流之间的转换。

- SHALL 使用 Length-prefixed 分帧（4 字节大端序长度头 + JSON 消息体）
- SHALL 支持 `StreamMessage<T>` 的泛型反序列化（通过 Jackson TypeReference）
- SHALL 处理半包/粘包问题（Netty ByteToMessageDecoder 机制）

#### Scenario: 编码消息
- **WHEN** 将 `StreamMessage<String>` 编码为字节
- **THEN** 输出为 `[4字节长度][JSON字节]` 格式

#### Scenario: 解码消息
- **WHEN** 收到 `[0x00,0x00,0x00,0x12, ...JSON...]` 字节流
- **THEN** 解码为对应的 `StreamMessage<String>` 对象

### Requirement: 客户端实现

系统 SHALL 实现 `NettyStreamClient`，基于 `StreamClient` 接口。

- SHALL 使用 `Bootstrap` + `QuicChannel` 建立 QUIC 连接
- SHALL 管理连接生命周期（connect → active → close）
- SHALL 支持 `newCall` 创建 `DefaultStreamCall` 实例
- SHALL 在 `shutdown()` 时优雅关闭所有活跃流

#### Scenario: 客户端连接并发送消息
- **WHEN** 客户端连接到服务端，发送一条 DATA 消息，收到回复后关闭
- **THEN** 整个流程走通，无异常

### Requirement: 服务端实现

系统 SHALL 实现 `NettyStreamServer`，基于 `StreamServer` 接口。

- SHALL 使用 `QuicServer` 监听 UDP 端口
- SHALL 维护 `Map<String, StreamHandler>` 路径到处理函数的路由表
- SHALL 在收到新 QUIC stream 时，根据 path 分发到对应 handler
- SHALL 支持优雅关闭

#### Scenario: 服务端接收并处理消息
- **WHEN** 服务端注册 `/echo` 处理函数，客户端连接并发送消息
- **THEN** 服务端接收消息，处理函数被调用，响应发送回客户端

### Requirement: StreamHandler 处理函数

系统 SHALL 定义 `StreamHandler<ReqT, RespT>` 函数式接口，处理流式请求。

- 入参为 `StreamReceiver<ReqT>`（接收请求）和 `StreamSender<RespT>`（发送响应）
- 处理函数内可异步或同步处理消息

#### Scenario: Echo 处理函数
- **WHEN** 注册 handler 为 `(req, resp) -> req.onMessage(msg -> resp.send(msg))`
- **THEN** 服务端收到客户端消息后原样回复