# HTTP/3 Streaming Demo

基于 Java 17 + Maven + Netty QUIC 的 HTTP/3 双向流式消息通信示例项目。

## 项目结构

```
http3-demo/
├── pom.xml                     # 父 POM（多模块）
├── http3-api/                  # API 接口层
│   └── src/main/java/io/http3/demo/api/
│       ├── StreamMessage.java      # 泛型消息模型
│       ├── StreamMessageType.java  # 消息类型枚举
│       ├── StreamMessageCodec.java # JSON 编解码工具
│       ├── StreamSender.java       # 发送接口
│       ├── StreamReceiver.java     # 接收接口
│       ├── StreamCall.java         # 双向流调用
│       ├── StreamClient.java       # 客户端接口
│       ├── StreamServer.java       # 服务端接口
│       └── StreamHandler.java      # 处理函数接口
├── http3-core/                 # 核心实现
│   └── src/main/java/io/http3/demo/core/
│       ├── codec/                  # 消息编解码器
│       │   ├── StreamMessageCodec.java
│       │   ├── StreamMessageEncoder.java
│       │   └── StreamMessageDecoder.java
│       └── impl/                   # 默认实现
│           ├── DefaultStreamCall.java
│           ├── NettyStreamClient.java
│           ├── NettyStreamServer.java
│           └── StreamDispatchHandler.java
└── http3-example/              # 示例程序
    └── src/main/java/io/http3/demo/example/
        ├── echo/                   # Echo 回声
        ├── chat/                   # Chat 聊天
        └── streamdata/            # 流式数据推送
```

## 快速开始

### 构建

```bash
./mvnw clean package -DskipTests
```

### 运行示例

#### 1. Echo 回声示例

服务端：
```bash
./mvnw exec:java -pl http3-example -Dexec.mainClass="io.http3.demo.example.echo.EchoServer"
```

客户端（新终端）：
```bash
./mvnw exec:java -pl http3-example -Dexec.mainClass="io.http3.demo.example.echo.EchoClient" -Dexec.args="localhost 9999 HelloHTTP3"
```

#### 2. Chat 聊天示例

服务端：
```bash
./mvnw exec:java -pl http3-example -Dexec.mainClass="io.http3.demo.example.chat.ChatServer"
```

客户端：
```bash
./mvnw exec:java -pl http3-example -Dexec.mainClass="io.http3.demo.example.chat.ChatClient" -Dexec.args="localhost 9999 HelloWorld"
```

#### 3. StreamData 流式数据推送

服务端：
```bash
./mvnw exec:java -pl http3-example -Dexec.mainClass="io.http3.demo.example.streamdata.StreamDataServer"
```

客户端：
```bash
./mvnw exec:java -pl http3-example -Dexec.mainClass="io.http3.demo.example.streamdata.StreamDataClient" -Dexec.args="localhost 9999"
```

## API 概览

### 消息模型

```java
StreamMessage<String> msg = StreamMessage.data("hello");
msg.type();    // StreamMessageType.DATA
msg.payload(); // "hello"
msg.msgId();   // UUID
```

### 双向流调用

```java
// 客户端
StreamClient client = new NettyStreamClient();
client.connect("localhost", 9999);

StreamCall<String, String> call = client.newCall("/echo", String.class, String.class);

call.responseStream()
    .onMessage(msg -> System.out.println("收到: " + msg.payload()))
    .onComplete(() -> System.out.println("流完成"))
    .onError(cause -> System.err.println("错误: " + cause));

call.requestStream().send(StreamMessage.data("Hello!"));
call.requestStream().complete();
call.await();
```

### 服务端

```java
StreamServer server = new NettyStreamServer();
server.registerService("/echo", (request, response) -> {
    request.onMessage(msg -> response.send(msg));
    request.onComplete(() -> response.complete());
});
server.start(9999);
```

## 技术栈

| 组件 | 版本 |
|------|------|
| Java | 17 |
| Maven | 3.9.8+ |
| Netty | 4.1.115.Final |
| Netty QUIC | 0.0.20.Final |
| Jackson | 2.17.2 |
| JUnit | 5.11.0 |

## 协议

消息帧格式（Length-prefixed JSON）：

```
[4字节: 消息体长度(大端序)] [N字节: JSON消息体]
```

JSON 消息体结构：

```json
{
  "msgId": "uuid",
  "type": "DATA|COMPLETE|ERROR|PING",
  "payload": {...}
}
```

## 构建验证

```bash
# 单元测试
./mvnw test

# 跳过测试打包
./mvnw package -DskipTests

# 完整构建
./mvnw clean install -DskipTests
```