## Why

QUIC 是基于 UDP 的新一代传输协议，实现多路复用、0-RTT 握手、连接迁移等特性，特别适合低延迟、高并发的流式通信场景。当前 Java 生态中缺乏一个简洁、可复用的 QUIC 双向流式消息通信的 demo 和封装层，本项目旨在填补这一空白，提供一个开箱即用的示例和 API 封装。

## What Changes

- 新建 Maven 多模块项目 `quic-stream-demo`，基于 Java 17
- 创建 `quic-stream-api` 模块：定义流式消息通信的通用接口（StreamMessage、StreamSender、StreamReceiver、StreamCall、StreamClient、StreamServer）
- 创建 `quic-stream-core` 模块：基于 Netty + `netty-incubator-codec-quic` 实现 QUIC 通信，包含消息编解码、Netty 集成、接口默认实现
- 创建 `quic-stream-example` 模块：提供三个示例场景（Echo、Chat、StreamData）
- 引入 Maven Wrapper（mvnw），无需系统安装 Maven
- 使用自签名 TLS 证书支持 QUIC+TLS 1.3 握手

## Capabilities

### New Capabilities
- `stream-api`: 定义泛型化的流式消息通信接口，包括单向/双向流、消息模型、生命周期回调
- `stream-core`: 基于 Netty QUIC 的通信核心实现，支持消息编解码、流管理、连接管理
- `stream-example`: 三个示例场景，演示 API 的使用方式

### Modified Capabilities

无（新项目，无已有能力需修改）

## Impact

- 新增约 20-30 个 Java 源文件，分布在三个 Maven 模块中
- 新增依赖：`netty-incubator-codec-quic`、`netty-handler`、`jackson-databind`、`slf4j`
- 新增 Maven Wrapper 脚本
- 生成自签名证书用于 QUIC TLS 握手
- 无已有代码影响（全新项目）