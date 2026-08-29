## 1. 项目初始化

- [x] 1.1 创建父 POM 文件（pom.xml），配置多模块、Java 17、Netty QUIC 依赖版本
- [x] 1.2 安装 Maven Wrapper（mvnw），验证 mvnw --version
- [x] 1.3 创建 http3-api 模块（pom.xml + 空目录结构）
- [x] 1.4 创建 http3-core 模块（pom.xml + 空目录结构）
- [x] 1.5 创建 http3-example 模块（pom.xml + 空目录结构）
- [x] 1.6 生成自签名 TLS 证书（keytool 脚本，输出到 http3-core/src/main/resources/certs/）

## 2. 实现 StreamMessage 模型

- [x] 2.1 实现 `StreamMessage<T>` 类（msgId、type 枚举、payload 泛型）
- [x] 2.2 实现 `StreamMessageType` 枚举（DATA、COMPLETE、ERROR、PING）
- [x] 2.3 实现 StreamMessage 的 Jackson 序列化/反序列化（含泛型 TypeReference 支持）

## 3. 实现 API 接口层

- [x] 3.1 实现 `StreamSender<T>` 接口
- [x] 3.2 实现 `StreamReceiver<T>` 接口
- [x] 3.3 实现 `StreamCall<ReqT, RespT>` 接口
- [x] 3.4 实现 `StreamClient` 接口
- [x] 3.5 实现 `StreamServer` 接口
- [x] 3.6 实现 `StreamHandler<ReqT, RespT>` 函数式接口

## 4. 实现消息编解码器

- [x] 4.1 实现 `StreamMessageEncoder`（Netty MessageToByteEncoder，Length-prefixed 分帧）
- [x] 4.2 实现 `StreamMessageDecoder`（Netty ByteToMessageDecoder，半包/粘包处理）
- [x] 4.3 实现 `StreamMessageCodec` 组合编解码器
- [x] 4.4 编写编解码器单元测试

## 5. 实现 NettyStreamClient

- [x] 5.1 实现 `NettyStreamClient`（Bootstrap + QuicChannel 建立 QUIC 连接）
- [x] 5.2 实现 `connect()` 方法（含 TLS 配置、EventLoopGroup 初始化）
- [x] 5.3 实现 `newCall()` 方法（创建 QUIC 双向流，绑定编解码器）
- [x] 5.4 实现 `shutdown()` 方法（优雅关闭）

## 6. 实现 NettyStreamServer

- [x] 6.1 实现 `NettyStreamServer`（QuicServerCodecBuilder 监听 UDP 端口）
- [x] 6.2 实现 `registerService()` 方法（路由表维护）
- [x] 6.3 实现 `start()` 方法（含端口绑定、TLS 配置）
- [x] 6.4 实现 `stop()` 方法（优雅关闭）
- [x] 6.5 实现服务端 `StreamDispatchHandler`（根据 path 分发到 StreamHandler）

## 7. 实现 DefaultStreamCall

- [x] 7.1 实现 `DefaultStreamCall<ReqT, RespT>`（绑定 QUIC 双向 stream 到 request/response 流）
- [x] 7.2 实现请求/响应生命周期管理（连接、消息传递、关闭）

## 8. 实现示例程序

- [x] 8.1 实现 EchoServer（注册 `/echo` handler，原样返回消息）
- [x] 8.2 实现 EchoClient（连接、发送消息、接收回复）
- [x] 8.3 实现 ChatServer（注册 `/chat` handler，处理 JOIN/MESSAGE/LEAVE）
- [x] 8.4 实现 ChatClient（连接、输入消息、接收回复）
- [x] 8.5 实现 StreamDataServer（注册 `/stream-data` handler，定时推送数据）
- [x] 8.6 实现 StreamDataClient（请求数据流、接收推送、取消）

## 9. 集成测试与验证

- [x] 9.1 编写 Echo 集成测试（已通过）
- [x] 9.2 编写 Chat 集成测试（已通过，含多客户端测试）
- [x] 9.3 编写 StreamData 集成测试（已通过，含取消测试）
- [x] 9.4 验证 mvnw clean install 构建通过
- [x] 9.5 编写 README 项目文档（含构建、运行命令）