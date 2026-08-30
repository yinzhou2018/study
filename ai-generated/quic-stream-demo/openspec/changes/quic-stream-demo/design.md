## Context

本项目是一个全新的 QUIC 流式通信示例项目，零历史代码。项目需基于 Java 17 + Maven 构建，底层使用 Netty + `netty-incubator-codec-quic` 实现 QUIC 协议栈，上层封装泛型化的双向流式消息通信 API。

当前环境：JDK 17 (TencentKonaJDK)，macOS aarch64，Maven 通过 mvnw wrapper 管理。

## Goals / Non-Goals

**Goals:**
- 提供一套简洁的泛型流式消息通信接口（StreamMessage、StreamSender、StreamReceiver、StreamCall、StreamClient、StreamServer）
- 基于 Netty QUIC 实现 QUIC 客户端和服务端
- 支持 JSON 序列化的消息编解码（Length-prefixed 分帧）
- 提供三个示例场景：Echo（回声）、Chat（双向聊天）、StreamData（服务端推送）
- 使用 mvnw wrapper，零额外环境依赖

**Non-Goals:**
- 不追求生产级性能调优
- 不覆盖 QUIC 连接迁移、0-RTT 重放保护等高级特性
- 不实现服务发现、负载均衡
- 不做 gRPC 兼容

## Decisions

### 1. 底层栈：Netty + netty-incubator-codec-quic

**选择原因：**
- Netty 是 Java 最成熟的 NIO 框架，QUIC codec 在 incubator 项目中持续活跃
- 对比方案：Jetty HTTP/3 起步较晚且 API 不够灵活；Vert.x 基于 Netty 但引入额外抽象层
- 原生控制 QUIC stream 粒度，便于实现双向流式消息

### 2. 序列化：JSON (Jackson)

**选择原因：**
- 相比 Protobuf：demo 项目可读性优先，JSON 无需 schema 编译步骤
- 相比纯文本：有类型安全，可扩展
- Jackson 是 Java 生态事实标准，零学习成本

### 3. 分帧：Length-prefixed（4 字节长度头 + 消息体）

**选择原因：**
- 相比行分隔符：不需要转义，支持二进制安全
- 相比自描述：解析效率高，O(1) 定位消息边界
- 4 字节长度头支持最大 4GB 消息体

### 4. 流控模型：回调 + CompletableFuture

**选择原因：**
- 相比完全 Reactive（Project Reactor）：回调模型更直观，demo 的受众更广
- 相比纯阻塞：CompletableFuture 提供异步编排能力
- 回调接口易于二次封装为 Reactive 或 Virtual Thread 模型

### 5. Maven 多模块架构

```
quic-stream-demo (parent pom)
├── quic-stream-api      — 纯接口模块，零依赖，可独立发布
├── quic-stream-core     — 实现模块，依赖 quic-stream-api + Netty
└── quic-stream-example  — 示例模块，依赖 quic-stream-core
```

### 6. 自签名 TLS 证书

- 使用 keytool 在构建时生成自签名证书
- QUIC 基于 TLS 1.3，示例使用 `InsecureTrustManagerFactory` 跳过客户端验证（demo 用途）
- 示例使用 `InsecureTrustManagerFactory` 跳过客户端验证（demo 用途）

## Messaging Protocol

```
流生命周期:
  [Conn Open] → [Stream Open] → [Message*] → [Stream Close] → [Conn Close]

消息帧格式（wire format）:
  ┌─────────────────────────────────────┐
  │   4 bytes: Payload Length (big-end) │
  ├─────────────────────────────────────┤
  │   N bytes: JSON Payload             │
  │   {                                 │
  │     "msgId": "uuid",                │
  │     "type": "DATA" | "COMPLETE" |   │
  │             "ERROR" | "PING",       │
  │     "payload": {...}                │
  │   }                                 │
  └─────────────────────────────────────┘

QUIC Stream 映射:
  Client → Server: 每条消息一个 QUIC stream (unidirectional)
  Server → Client: 每条消息一个 QUIC stream (unidirectional)
  或: 双向 stream 复用 (bidirectional)
  
  方案: 使用双向 QUIC stream 实现 StreamCall 的请求/响应配对
```

## Risks / Trade-offs

| 风险 | 缓解措施 |
|------|---------|
| netty-incubator-codec-quic 在 macOS aarch64 的 native 库兼容性 | 锁定已知兼容版本，如果在 macOS 上 native 加载失败，增加 fallback 提示 |
| QUIC 在本地开发环境可能被防火墙阻止（UDP 端口） | 示例使用 9999 端口，提供 `curl` 检查命令 |
| Maven Wrapper 首次下载需要网络 | 仅在首次构建时有损耗 |
| 自签名证书导致浏览器/工具不信任 | Demo 范围，文档说明如何添加信任或跳过验证 |
| Netty QUIC native 库下载可能被 GFW 限速 | 建议配置阿里云 Maven 镜像，提供 proxy 配置说明 |