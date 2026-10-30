# Seeduplex 接入接口文档与使用说明

> 适用范围：豆包实时语音模型 3.0（Seeduplex）全双工端到端实时语音接口。  
> 信息来源：火山引擎官方文档《端到端实时语音（Seeduplex）》与《接入必读》。  
> 整理日期：2026-10-30。

源文档：

- 接口说明：<https://docs.volcengine.com/docs/doubao-voice/end-to-end-real-time-voice-full-duplex-version?lang=zh>
- 接入必读：<https://docs.volcengine.com/docs/doubao-voice/access-must-read?lang=zh>

官方示例 demo（下载后可参考完整工程实现）：

- Go 示例：`go1.24_duplex_demo.zip`
- Python 示例：`python3.7_duplex_demo.zip`
- Web 页面快速体验 demo：`web_duplex_demo.zip`（下载后输入 API Key 即可在页面中可视化操作并体验全双工效果）

---

## 目录

- [1. 概述](#1-概述)
- [2. 接入准备](#2-接入准备)
- [3. 协议总览](#3-协议总览)
- [4. 音频约束](#4-音频约束)
- [5. 建立会话 session.create](#5-建立会话-sessioncreate)
- [6. session 参数详解](#6-session-参数详解)
- [7. extension 专有能力配置](#7-extension-专有能力配置)
- [8. 更新会话 session.update](#8-更新会话-sessionupdate)
- [9. 上行事件总表](#9-上行事件总表)
- [10. 发送音频流 input_audio_buffer.append](#10-发送音频流-input_audio_bufferappend)
- [11. 强制判停 input_audio_buffer.commit](#11-强制判停-input_audio_buffercommit)
- [12. 静音保活 input_audio_mute.commit / input_audio_unmute.commit](#12-静音保活-input_audio_mutecommit--input_audio_unmutecommit)
- [13. 打招呼 speech_text_buffer.commit](#13-打招呼-speech_text_buffercommit)
- [14. 干预模型回复 speech_text_buffer.replacement.append / commit](#14-干预模型回复-speech_text_bufferreplacementappend--commit)
- [15. 客户端打断 response.cancel](#15-客户端打断-responsecancel)
- [16. 结束会话 session.close](#16-结束会话-sessionclose)
- [17. 上下文管理](#17-上下文管理)
- [18. Function Calling](#18-function-calling)
- [19. 下行事件总表](#19-下行事件总表)
- [20. 下行事件示例](#20-下行事件示例)
- [21. 错误码](#21-错误码)
- [22. 稳定性与限流](#22-稳定性与限流)
- [23. 接口迁移与事件对照表](#23-接口迁移与事件对照表)
- [24. 视频帧采集最佳实践（Doubao-Seed-Realtime-1.0）](#24-视频帧采集最佳实践doubao-seed-realtime-10)
- [25. 推荐接入流程](#25-推荐接入流程)
- [26. 最小实现示例（Python）](#26-最小实现示例python)
- [27. 开发注意事项](#27-开发注意事项)

---

## 1. 概述

豆包实时语音模型 3.0（**Seeduplex**）是一款**全双工端到端语音大模型**，支持低延迟、多模态、语音到语音（S2S）的实时对话能力。

Seeduplex 采用全新的 **Realtime 事件协议**，与原版本差异较大。接入前请务必完整阅读本文档（尤其是协议设计原则、音频约束与最佳实践），避免因协议不熟悉导致接入异常。

核心能力：

- 全双工实时语音对话，低时延、高拟人。
- 标准化 Realtime 事件语义，纯 JSON 文本帧收发。
- 通过 `extension` 透传模型专有能力：唱歌、联网搜索、克隆音色、热词、上下文初始化等。
- 支持 Function Calling、客户端打断、上下文管理、干预回复等。

---

## 2. 接入准备

### 2.1 服务地址

Seeduplex 通过 WebSocket 连接：

```text
wss://openspeech.bytedance.com/api/v3/duplex/realtime/dialogue
```

所有上下行数据均以 **JSON 文本帧** 发送，一条 WebSocket Text Message 只承载一个完整 JSON 事件。

### 2.2 鉴权（请求头）

新版控制台接入使用请求头：

| 字段 | 类型 | 必选 | 说明 |
| --- | --- | --- | --- |
| `X-Api-Key` | string | 是 | API Key，可从 **控制台 > API Key 管理** 获取。 |

注意：

- 同时兼容旧版控制台的鉴权方式，详见旧版控制台鉴权参考示例。
- 旧版控制台后续会逐步下线，**建议尽快切换至新版控制台使用 `X-Api-Key`**。
- 浏览器端 WebSocket 无法自定义请求头，建议通过后端代理建立到 Seeduplex 的 WSS 连接，由后端注入 `X-Api-Key`，避免将 API Key 暴露在前端。

### 2.3 响应头

| 字段 | 说明 |
| --- | --- |
| `X-Tt-Logid` | 服务端返回的 Logid，用于咨询或反馈时定位问题。建议在日志中记录每个下行事件的该头部。 |

### 2.4 连接与用量限制

- **QPM**：每分钟 `StartSession` / `session.create` 调用次数，默认 `60`。
- **TPM**：每分钟 token 消耗量，默认 `100000`（10 万）。
- 限制按 **AppID / 项目维度** 计算。
- 服务端会因超过 **10 分钟无交互** 而释放连接。
- 若预估用量将超过配额，请提前联系对应商务沟通扩容。

---

## 3. 协议总览

### 3.1 协议设计原则

核心设计理念：**事件语义标准化，专有能力通过 `extension` 透传**。

- **标准化事件命名**：上行事件包括 `session.create`、`input_audio_buffer.append`、`conversation.item.create`、`response.cancel` 等；下行事件包括 `session.created`、`response.output_audio.delta`、`response.done` 等。
- **结构化字段组织**：`session` 对象内含 `audio.input`、`audio.output`、`instructions`、`tools` 等字段，层次清晰、便于扩展。
- **专有能力通过 `extension` 透传**：唱歌、联网搜索、克隆音色、热词、上下文初始化等模型专有能力，统一收敛至 `extension`（包含 `asr`、`tts`、`dialog`），其内部结构与原端到端模型的 `StartSessionPayload` 完全一致，**存量配置可直接复用**。
- **纯 JSON 文本帧**：所有事件均以 JSON 文本通过 WebSocket Text Message 收发，事件类型由 `type` 字段标识；音频数据以 Base64 编码内嵌于对应事件的 `audio` / `delta` 字段中。

### 3.2 消息帧格式

协议采用纯 JSON 文本帧传输：每个事件均为一条 JSON 字符串，通过 WebSocket Text Message 收发，无需二进制封装。

| 约定 | 说明 |
| --- | --- |
| **一帧一事件** | 每条 WebSocket Text Message 承载且仅承载一个完整事件 JSON。 |
| **type 路由** | 客户端与服务端均按顶层 `type` 字段（如 `session.create`、`response.output_audio.delta`）进行分发处理。 |
| **event_id（可选）** | 上行事件可在 `event_id` 字段携带客户端生成的幂等标识；服务端会在相关下行事件中回带，便于链路追踪与排障。 |
| **session 标识** | 会话建立后，服务端通过 `session.created` 下行返回 `session.id`（即 `dialog.id`）；接续历史会话时，在 `session.update` 的 `session.id` 字段中回传。 |
| **音频承载方式** | 音频不走独立的二进制通道，而是以 Base64 编码字符串的形式内嵌于 JSON 事件字段中。 |

示例：

```json
{
  "type": "session.create",
  "session": {
    "model": "1.2.6.1"
  }
}
```

---

## 4. 音频约束

### 4.1 输入音频

**格式约束**：

- 格式：`pcm` 或 `speech_opus`（由服务端自动转为 PCM）。
- 声道：单声道。
- 采样率：`16000 Hz`。
- 位深：`int16`，小端序。
- 推荐分片：**20 ms 一包**（16k / int16 格式下每包 640 字节）。
- 音频字节经 Base64 编码后放入 `input_audio_buffer.append.audio` 字段。

**质量约束**：

| 指标 | 推荐范围 | 最低准入 |
| --- | --- | --- |
| 人声 RMS（纯人声部分） | `-31.7 ~ -23.9 dBFS`，中心目标约 `-27.6 dBFS` | 不低于 `-37 dBFS`（低于此值将影响模型效果） |
| 人声峰值 | — | 不高于 `-19.5 dBFS`，峰值应控制在 `≤ -1 dBFS` |
| 背景噪声 RMS（纯背景噪声部分） | 不高于 `-50 dBFS`，中心目标约 `-52.6 dBFS` | 不高于 `-45 dBFS` |
| 信噪比 SNR | `20~30 dB` | 不低于 `15 dB` |

> **注意**：
> - 音频分片建议以 20 ms 为单位（16k / int16 格式下每包 640 字节），并**严格按照实时音频流的节奏发送**，发送速率偏离实际节奏（过快或过慢）都将触发服务端错误。
> - 模型依赖上行音频流保活。客户端关闭麦克风后若不再发送音频帧，须主动发送静音事件 `input_audio_mute.commit`，恢复麦克风时发送取消静音事件 `input_audio_unmute.commit`，否则服务端会因持续收不到音频输入触发超时，导致模型无响应。
> - 人声过轻或背景噪声过大均会导致效果下降，请尽量将输入音频调整至上述推荐范围内。

### 4.2 输出音频

- 在 `session.audio.output.format` 中配置输出音频格式，默认返回 Ogg Opus 格式的音频。
- 采样率仅支持 `24000 Hz`。
- 音频数据以 Base64 编码在 `response.output_audio.delta.delta` 字段下行。

支持的输出格式：

| 格式 | 说明 |
| --- | --- |
| `pcm` | 32 位小端序 PCM 格式 |
| `pcm_s16le` | 16 位小端序 PCM 格式 |
| `ogg_opus` | Ogg Opus 压缩格式（默认推荐） |

---

## 5. 建立会话 session.create

首次接入时发送 `session.create`。模型 ID 固定为：

```text
1.2.6.1
```

### 5.1 输入示例

```json
{
  "type": "session.create",
  "session": {
    "id": "对应 dialog id",
    "model": "1.2.6.1",
    "instructions": "You are a creative assistant that helps with design tasks.",
    "audio": {
      "input": {
        "format": {
          "type": "pcm",
          "rate": 16000
        }
      },
      "output": {
        "format": {
          "type": "pcm",
          "rate": 24000
        },
        "speed": 0,
        "loudness": 0,
        "voice": "zh_female_vv_jupiter_bigtts"
      }
    },
    "tools": [
      {
        "type": "function",
        "name": "display_color_palette",
        "description": "Call this function when a user asks for a color palette.",
        "parameters": {
          "type": "object",
          "properties": {
            "theme": {
              "type": "string",
              "description": "Description of the theme."
            },
            "colors": {
              "type": "array",
              "description": "Five hex codes.",
              "items": {
                "type": "string"
              }
            }
          },
          "required": ["theme", "colors"]
        }
      }
    ]
  },
  "extension": {
    "asr": {},
    "tts": {},
    "dialog": {}
  }
}
```

### 5.2 输出示例

```json
{
  "type": "session.created",
  "event_id": "event_C9G5RJeJ2gF77mV7f2B1j",
  "session": { "id": "对应 dialog id" }
}
```

收到 `session.created` 后，返回的 `session.id`（对应原版本 `dialog.id`）可用于接续历史对话内容，随后可继续发送音频流。

---

## 6. session 参数详解

### 6.1 `session`

| 字段 | 类型 | 必选 | 说明 |
| --- | --- | --- | --- |
| `id` | string | 否 | 对应原 `dialog id`，接续历史对话时传入。 |
| `model` | string | 是 | 固定值 `1.2.6.1`。 |
| `instructions` | string | 否 | 系统提示词（System message），用于引导模型的回复内容与音频风格。 |
| `audio` | object | 是 | 输入与输出音频配置。 |
| `tools` | array | 否 | Function Calling 工具定义，使用标准 JSON Schema 结构（`type=function` + `parameters`）。 |

> `instructions` + 模型内部 SP + 对话历史上下文 + FC 结果，合计长度上限为 **64K tokens**。

### 6.2 `session.audio.input`

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `format.type` | string | 输入音频格式，支持 `pcm` 和 `speech_opus`。 |
| `format.rate` | int | 输入采样率，仅支持 `16000`，单位 Hz。 |

### 6.3 `session.audio.output`

| 字段 | 类型 | 必选 | 说明 |
| --- | --- | --- | --- |
| `format.type` | string | 是 | 输出音频格式，支持 `pcm`、`pcm_s16le`、`ogg_opus`。 |
| `format.rate` | int | 是 | 输出采样率，仅支持 `24000`，单位 Hz。 |
| `voice` | string | 是 | 音色 ID。支持官方音色（详见音色列表）和复刻音色。 |
| `speed` | number | 否 | 语速。默认 `0`，范围 `[-50,100]`，数值越大越快。`-50` 为 0.5 倍速，`100` 为 2.0 倍速。 |
| `loudness` | number | 否 | 音量。默认 `0`，范围 `[-50,100]`，数值越大越大。`-50` 为 0.5 倍音量，`100` 为 2.0 倍音量。 |

---

## 7. extension 专有能力配置

`extension` 用于透传模型专有能力，包含 `asr`、`tts`、`dialog` 三个子对象。其内部结构与原端到端模型的 `StartSessionPayload` 完全一致，存量配置可直接复用。

### 7.1 `extension.asr`

配置 ASR 参数。

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `extra.enable_asr_twopass` | bool | 启用非流式模型识别能力，默认 `false`。 |
| `extra.boosting_table_id` | string | 热词词表 ID，可在控制台 > 自学习平台配置热词后获取。**需同时将 `enable_asr_twopass` 设为 `true`，否则无法生效。** |
| `extra.boosting_table_name` | string | 热词词表名称。**需同时将 `enable_asr_twopass` 设为 `true`。** |
| `extra.regex_correct_table_id` | string | 正则替换词表 ID。相较于替换词的精确匹配替换，正则替换词适合批量格式转换（如日期格式统一、符号标准化）、模糊模式匹配等复杂场景。 |
| `extra.regex_correct_table_name` | string | 正则替换词表名称。 |
| `extra.context` | string | ASR 上下文信息。 |

### 7.2 `extension.dialog`

配置 Dialog 参数。

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `location` | string | 位置信息配置，支持字段：`longitude`（经度）、`latitude`（纬度）、`city`（城市）、`country`（国家）、`province`（省份）、`district`（区县）、`town`（乡镇 / 街道）、`country_code`（国家代码）、`address`（详细地址）。 |
| `dialog_context` | array | 初始化上下文，需按照 `user` / `assistant` 成对传入 QA。 |
| `extra.strict_audit` | bool | 开启严格审核，默认 `true`。`true` 严格审核，`false` 普通审核。 |
| `extra.audit_response` | string | 指定用户 query 命中安全审核时的自定义回复话术。 |
| `extra.enable_volc_websearch` | bool | 开启内置联网功能，默认 `false`。开通服务请参考控制台融合信息搜索 API。 |
| `extra.volc_websearch_type` | string | 指定搜索服务类型，支持 `web_custom_api`、`web_global_api`。 |
| `extra.volc_websearch_api_key` | string | 融合信息搜索 API 密钥，开启 `enable_volc_websearch` 后需配置。 |
| `extra.enable_music` | bool | 开启唱歌能力，默认 `false`。开启后系统将从曲库检索唱歌数据并传入模型。 |
| `extra.enable_loudness_norm` | bool | 开启输出音频响度均衡能力，默认 `false`。 |
| `extra.enable_user_query_exit` | bool | 开启退出意图识别能力，默认 `false`。开启后，服务端将在 `response.output_audio.done` 事件中携带退出意图信号，供客户端执行退出操作。 |

`dialog_context` 条目结构：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `role` | string | 对话角色：`user` / `assistant`。 |
| `text` | string | 对话文本。 |
| `timestamp` | int | 对话时间戳。 |

> **搜索服务类型选择建议**：
> - `web_custom_api`（豆包搜索 Custom 版）：面向行业定制的联网搜索。响应更快（相比 Global 版快约 350ms），单请求返回条数更多，支持行业领域与权威分级筛选，可直接输出网页完整正文。适用于具有明确行业属性的高频调用场景，尤其适合对响应时延敏感的业务。
> - `web_global_api`（豆包搜索 Global 版）：面向全球站点的通用联网搜索。覆盖面更广，支持全球内容检索，支持摘要长度自由调节与网页原文插图返回，综合搜索效果更优。适用于跨境内容检索及对结果质量、摘要可读性要求较高的场景。

> **唱歌能力注意**：当前仅官方主推的 4 个音色（Vivi 2.0、小何 2.0、云舟 2.0、小天 2.0）可保证较好的歌唱合成效果。

### 7.3 `extension.tts`

配置 TTS 参数。

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `extra.max_length_to_filter_parenthesis` | int | 过滤括号内文本的长度，单位为字符，默认 `0`（即不过滤），推荐取值范围 `0~100`。当括号内文本字符数超过设定值时，该括号的过滤功能会失效，括号内内容会被正常朗读。 |
| `extra.explicit_dialect` | string | 指定方言参数。支持：`dongbei`、`sichuan`、`shaanxi`、`yue`、`beijing`、`henan`、`tianjin`、`shanghai`。 |
| `extra.aigc_metadata.enable` | bool | 开启隐式水印，默认 `false`。 |
| `extra.aigc_metadata.content_producer` | string | 指定合成服务提供者的名称或编码。 |
| `extra.aigc_metadata.produce_id` | string | 指定内容制作编号。 |
| `extra.aigc_metadata.content_propagator` | string | 指定内容传播服务提供者的名称或编码。 |
| `extra.aigc_metadata.propagate_id` | string | 指定内容传播编号。 |

> **括号过滤说明**：该参数用于过滤文本中括号内的注释、补充说明等无需朗读的内容。若存在较长的括号文本需要过滤，建议客户端在送入合成前自行完成前置过滤，以避免过滤失效及不必要的时延。

> **方言支持说明**：当前仅 `vivi`、`小何`、`云舟` 和 `小天` 4 个音色支持方言参数，具体音色 ID 请参见音色列表。

---

## 8. 更新会话 session.update

会话过程中可发送 `session.update` 更新配置。字段结构与 `session.create` 的 `session` 基本一致，`type` 固定为 `session.update`。

```json
{
  "type": "session.update",
  "session": {
    "id": "上次 session.created 返回的 id",
    "instructions": "新的系统提示词",
    "tools": []
  }
}
```

服务端返回 `session.updated`（`session.update` 请求对应的确认响应 ack，表示会话配置已成功更新）。

> **注意**：
> - `session.update` 动态增删工具为**全量覆盖**，非增量更新。
> - 接续历史会话时，复用上次 `session.created` 返回的 `session.id`（即 `dialog.id`）即可接续，服务端默认保留最近 **20 轮问答记录**。

---

## 9. 上行事件总表

| 事件名 | 原事件名（旧版 S2S） | 用途 |
| --- | --- | --- |
| `session.create`（首次） | `StartSession` | 创建会话 |
| `session.update`（会话中） | `UpdateConfig` | 更新会话配置 |
| `session.close` | `FinishSession` | 结束会话 |
| `input_audio_buffer.append` | `TaskRequest` | 发送音频流 |
| `input_audio_buffer.commit` | `EndASR` | 强制判停 |
| `input_audio_mute.commit` | — | 静音保活（关闭麦克风时发送） |
| `input_audio_unmute.commit` | — | 取消静音（恢复麦克风时发送） |
| `speech_text_buffer.commit` | `SayHello` | 打招呼 |
| `speech_text_buffer.replacement.append` / `commit` | `ChatTTSText` | 干预模型回复 |
| `conversation.item.create` | `ConversationCreate` / `FunctionCallOutput` | 新增上下文 / FC 结果回传 |
| `conversation.item.update` | `ConversationUpdate` | 更新上下文 |
| `conversation.item.retrieve` | `ConversationRetrieve` | 查询上下文 |
| `conversation.item.delete` | `ConversationDelete` | 删除上下文 |
| `response.cancel` | `ClientInterrupt` | 客户端打断 |

> 下文第 10–16 节分别对每个上行事件给出 **接口定义、字段说明、使用场景、服务端响应与注意事项**。

---

## 10. 发送音频流 input_audio_buffer.append

### 10.1 接口定义

```json
{
  "event_id": "event_456",
  "type": "input_audio_buffer.append",
  "audio": "Base64EncodedAudioData"
}
```

### 10.2 字段说明

| 字段 | 类型 | 必选 | 说明 |
| --- | --- | --- | --- |
| `type` | string | 是 | 固定 `input_audio_buffer.append`。 |
| `event_id` | string | 否 | 客户端生成的幂等标识，服务端会在相关下行事件中回带，便于链路追踪。 |
| `audio` | string | 是 | Base64 编码后的音频数据。 |

### 10.3 使用场景

这是会话过程中**最核心、最高频**的上行事件，用于持续向服务端推送用户的实时语音流，驱动 ASR → 对话 → TTS 的完整响应链路。

典型实现：

1. 采集 16 kHz、单声道、int16 小端序 PCM。
2. 每 20 ms 取 640 字节为一个分片。
3. 对分片做 Base64 编码。
4. 作为 `audio` 字段发送 `input_audio_buffer.append`。

### 10.4 服务端响应

持续发送音频后，服务端会依次返回完整的下行响应链路（详见 [20. 下行事件示例](#20-下行事件示例)）：

```text
conversation.item.input_audio_transcription.started   (ASR 识别出首字)
conversation.item.input_audio_transcription.delta      (ASR 流式增量)
conversation.item.input_audio_transcription.completed  (ASR 识别完成)
response.output_text.delta                             (模型回复文本增量)
response.output_text.done                              (模型回复文本结束)
response.output_audio.started                          (TTS 合成开始)
response.output_audio.delta                            (TTS 流式音频块, Base64)
response.output_audio.done                             (TTS 一轮合成结束)
response.done                                          (本轮用量统计)
```

### 10.5 注意事项

- **严格按实时节奏发送**：发送速率偏离实际节奏（过快或过慢）都将触发服务端错误。
- **分片大小**：建议以 20 ms 为单位（16k / int16 格式下每包 640 字节）。
- **保活**：关闭麦克风后不再发送音频帧时，必须改发 `input_audio_mute.commit`，详见第 12 节。
- **音频质量**：人声过轻或背景噪声过大均会导致效果下降，请参照第 4 节音频约束。

---

## 11. 强制判停 input_audio_buffer.commit

### 11.1 接口定义

```json
{
  "event_id": "event_789",
  "type": "input_audio_buffer.commit"
}
```

### 11.2 字段说明

| 字段 | 类型 | 必选 | 说明 |
| --- | --- | --- | --- |
| `type` | string | 是 | 固定 `input_audio_buffer.commit`。 |
| `event_id` | string | 否 | 客户端生成的幂等标识，便于链路追踪。 |

该事件**不携带音频数据**，仅作为一个控制信号，通知服务端「当前用户的一句输入已经结束」。

### 11.3 使用场景

强制判停用于**主动结束一次用户音频输入**，对应原版本的 `EndASR`。典型场景包括：

- **用户明确说完一句话**：例如用户按下"发送/完成"按钮，或检测到明确的停顿意图，希望立即触发模型回复，而不等待模型自动判停。
- **模型未自动判停**：用户已停止说话，但模型迟迟未开始回复（VAD 未触发），此时主动 commit 可强制进入下一阶段。
- **需要手动结束一次输入**：在半双工交互模式或按键说话（PTT）场景下，松开按键即调用 commit 结束本轮输入。
- **切换交互对象**：需要打断当前输入流转入其他流程时，先用 commit 收尾。

### 11.4 服务端响应

服务端返回 `input_audio_buffer.committed`，表示输入音频缓冲区已成功提交，**标志着一次用户音频输入的结束**：

```json
{
  "event_id": "event_1121",
  "type": "input_audio_buffer.committed"
}
```

随后服务端会基于已提交的音频继续返回 ASR / Chat / TTS 下行事件（与 `input_audio_buffer.append` 触发的响应链路一致，详见 [20. 下行事件示例](#20-下行事件示例)）。

### 11.5 注意事项

- `commit` 只结束"输入"，不会取消已开始的回复；若要中断进行中的回复，请使用 `response.cancel`（第 15 节）。
- 在全双工模式下，模型通常能自动判停；`commit` 主要用于需要**主动、精确控制判停时机**的场景。
- 建议携带 `event_id`，便于在排障时与 `input_audio_buffer.committed` 及 `X-Tt-Logid` 关联。

---

## 12. 静音保活 input_audio_mute.commit / input_audio_unmute.commit

### 12.1 接口定义

关闭麦克风（暂停发送真实音频）时发送静音事件：

```json
{
  "type": "input_audio_mute.commit"
}
```

恢复麦克风时发送取消静音事件：

```json
{
  "type": "input_audio_unmute.commit"
}
```

### 12.2 使用场景

模型**依赖上行音频流保活**。在全双工会话中，服务端持续期待音频输入；若客户端关闭麦克风后长时间不再发送任何音频帧，服务端会因持续收不到音频输入触发**超时，导致模型无响应**，甚至因超过 10 分钟无交互而释放连接（错误码 `45000003`）。

因此：

- **关闭麦克风**（如用户主动静音、切换到其他任务、暂时离开）→ 立即发送 `input_audio_mute.commit`，通知服务端进入静音状态，不再期待实时音频。
- **恢复麦克风** → 发送 `input_audio_unmute.commit`，通知服务端恢复对音频输入的期待。

### 12.3 注意事项

- 这两个事件是**保活机制**，不是"发送静音音频帧"的替代；其作用是让服务端在不收到音频时不触发超时。
- 即便处于 mute 状态，会话仍需通过 `session.close` 正常关闭（第 16 节），不可直接断开 WebSocket。
- 若业务允许，也可在 mute 期间持续发送真实的静音 PCM 帧（全零 640 字节）作为另一种保活方式，但使用 `input_audio_mute.commit` 更轻量。

---

## 13. 打招呼 speech_text_buffer.commit

### 13.1 接口定义

```json
{
  "event_id": "evt_001",
  "type": "speech_text_buffer.commit",
  "text": "你好！"
}
```

### 13.2 字段说明

| 字段 | 类型 | 必选 | 说明 |
| --- | --- | --- | --- |
| `type` | string | 是 | 固定 `speech_text_buffer.commit`。 |
| `event_id` | string | 否 | 客户端生成的幂等标识，建议传入。 |
| `text` | string | 是 | 待合成的打招呼文本。 |

### 13.3 使用场景

打招呼用于**让模型主动说一段开场白**，对应原版本的 `SayHello`。典型场景：

- **会话开场白**：会话建立后（收到 `session.created`），主动播放一句欢迎语，如"你好，我是豆包，有什么可以帮你的吗？"，避免用户面对无声的初始状态。
- **引导用户开口**：在用户尚未说话时，主动提示交互方式。
- **固定话术播报**：需要由模型用配置音色合成并播报一段固定文本，而非让模型自行生成回复时。

> 与"干预模型回复"（第 14 节）的区别：打招呼是**直接指定文本并合成音频**，通常用于开场；干预回复则是在对话过程中替换/接管模型本来要生成的回复内容。

### 13.4 服务端响应

服务端会按 TTS 链路返回合成音频，依次为：

```json
// response.output_audio.started
{
  "event_id": "event_4",
  "type": "response.output_audio.started",
  "question_id": "question_001",
  "response_id": "",
  "tts_type": ""
}
// response.output_audio.delta
{
  "event_id": "event_4950",
  "type": "response.output_audio.delta",
  "question_id": "question_001",
  "response_id": "resp_001",
  "delta": "Base64EncodedAudioDelta"
}
// response.output_audio.done
{
  "event_id": "event_5152",
  "type": "response.output_audio.done",
  "question_id": "question_001",
  "response_id": "resp_001"
}
```

客户端收到 `response.output_audio.delta` 后解码 Base64 并播放即可。

### 13.5 注意事项

- 打招呼**不需要先发送音频输入**，可在 `session.created` 之后立即发送。
- `text` 内容会按 `session.audio.output` 配置的音色、语速、音量合成。
- 若需要在打招呼后转入正常对话，发送完 `speech_text_buffer.commit` 后即可继续 `input_audio_buffer.append` 推送用户音频。

---

## 14. 干预模型回复 speech_text_buffer.replacement.append / commit

### 14.1 接口定义

干预回复由**两个事件配合**完成：`append` 追加干预文本，`commit` 结束干预文本流。对应原版本的 `ChatTTSText`。

追加干预文本：

```json
// speech_text_buffer.replacement.append
{
  "event_id": "",
  "type": "speech_text_buffer.replacement.append",
  "text": "干预回复文本"
}
```

结束干预文本（结束包也支持带干预回复文本）：

```json
// speech_text_buffer.replacement.commit
{
  "event_id": "evt_001",
  "type": "speech_text_buffer.replacement.commit",
  "text": "（可选）结束包也支持带干预回复文本"
}
```

### 14.2 字段说明

| 字段 | 类型 | 必选 | 说明 |
| --- | --- | --- | --- |
| `type` | string | 是 | `speech_text_buffer.replacement.append` 或 `speech_text_buffer.replacement.commit`。 |
| `event_id` | string | 否 | 客户端生成的幂等标识。 |
| `text` | string | 是 | 干预回复文本。`append` 用于追加，`commit` 用于结束（也可携带一段文本）。 |

### 14.3 使用场景

干预模型回复用于**不让模型自行生成回复，而是由客户端直接指定回复文本**，再由服务端用配置音色合成音频。典型场景：

- **接管模型回复内容**：业务侧已通过其他逻辑（如规则匹配、人工坐席、外部知识库）确定了应当回复的话术，只需借模型的音色合成并播报，此时用 replacement 替换模型本来的回复。
- **强制播报特定话术**：例如安全审核命中后播报固定话术、特定意图下播报预设文案。
- **流式追加长文本**：通过多次 `append` 分段追加较长的干预文本，最后用 `commit` 收尾，服务端会据此合成完整音频。

### 14.4 服务端响应

服务端按 TTS 链路返回合成音频，**注意 `tts_type` 为 `chat_tts_text`**（与普通回复的 `tts_type` 不同，可用于区分本次音频来源于干预文本）：

```json
// response.output_audio.started
{
  "event_id": "event_4",
  "type": "response.output_audio.started",
  "question_id": "question_001",
  "response_id": "",
  "tts_type": "chat_tts_text"
}
// response.output_audio.delta
{
  "event_id": "event_4950",
  "type": "response.output_audio.delta",
  "question_id": "question_001",
  "response_id": "resp_001",
  "delta": "Base64EncodedAudioDelta"
}
// response.output_audio.done
{
  "event_id": "event_5152",
  "type": "response.output_audio.done",
  "question_id": "question_001",
  "response_id": "resp_001",
  "status_code": "20000002"
}
```

### 14.5 注意事项

- `append` 可多次发送以追加文本，`commit` 表示干预文本流结束；**必须发送 `commit`** 服务端才会完成本轮合成。
- `commit` 的 `text` 是可选的，结束包也支持带一段干预回复文本。
- 干预回复与打招呼（第 13 节）都直接指定文本合成音频，区别在于：打招呼用于开场，干预回复用于**替换/接管**对话过程中模型本要生成的回复。
- 括号内无需朗读的内容可配合 `extension.tts.extra.max_length_to_filter_parenthesis` 过滤，或由客户端在送入前自行前置过滤。

---

## 15. 客户端打断 response.cancel

### 15.1 接口定义

```json
{
  "type": "response.cancel"
}
```

### 15.2 字段说明

| 字段 | 类型 | 必选 | 说明 |
| --- | --- | --- | --- |
| `type` | string | 是 | 固定 `response.cancel`。 |

> 该事件本身可不携带 `event_id`；服务端返回的 `response.canceled` 会带 `event_id`。

### 15.3 使用场景

客户端打断用于**取消进行中的模型响应**，对应原版本的 `ClientInterrupt`。在全双工对话中是保证自然交互的关键能力。典型场景：

- **用户抢话 / 插话**：模型正在播报音频时，用户开始说话，客户端检测到用户语音后应立即 `response.cancel`，停止当前播报，转入新一轮听写，避免模型继续说完旧内容。
- **取消冗长回复**：模型回复偏离预期或过长，用户希望中止，主动触发打断。
- **切换话题**：用户改变意图，需要中断当前回复重新开始。
- **配合强制判停**：先 `response.cancel` 中断模型回复，再 `input_audio_buffer.commit` 结束新的用户输入，可实现"打断后立即接新输入"的流畅体验。

### 15.4 服务端响应

服务端返回 `response.canceled`，作为 `response.cancel` 请求的确认（ack）：

```json
{
  "type": "response.canceled",
  "event_id": "event_17"
}
```

收到该 ack 后，客户端可认为本轮响应已被取消，可安全开始下一轮交互。

### 15.5 注意事项

- `response.cancel` 取消的是**模型侧进行中的响应**（ASR/Chat/TTS 链路）；它不会结束会话，会话仍可继续。
- 打断后若要结束输入进入下一轮，需配合 `input_audio_buffer.commit`（第 11 节）。
- 打断后应停止本地音频播放队列的播放，避免残留音频继续播报。
- 要彻底结束会话请使用 `session.close`（第 16 节），不要用 `response.cancel` 代替。

---

## 16. 结束会话 session.close

### 16.1 接口定义

```json
{
  "event_id": "event_close",
  "type": "session.close"
}
```

### 16.2 字段说明

| 字段 | 类型 | 必选 | 说明 |
| --- | --- | --- | --- |
| `type` | string | 是 | 固定 `session.close`。 |
| `event_id` | string | 否 | 客户端生成的幂等标识。 |

### 16.3 使用场景

会话结束时发送，对应原版本的 `FinishSession`。用于正常关闭一轮对话会话、释放服务端资源。

### 16.4 服务端响应

```json
{
  "type": "session.closed"
}
```

### 16.5 注意事项

> **务必先发送 `session.close` 并收到服务端回复 `session.closed` 后，再关闭 WebSocket 链接**；直接断开会触发 `ContextCanceled` 错误（错误码 `55000001`）。

---

## 17. 上下文管理

Seeduplex 支持通过 `conversation.item` 系列事件管理历史上下文。

### 17.1 新增上下文 conversation.item.create

```json
{
  "type": "conversation.item.create",
  "items": [
    {
      "id": "111",
      "type": "message",
      "role": "user",
      "content": [
        { "type": "input_text", "text": "hi" }
      ]
    },
    {
      "id": "222",
      "type": "message",
      "role": "assistant",
      "content": [
        { "type": "input_text", "text": "hi!" }
      ]
    }
  ]
}
```

服务端返回：

```json
// conversation.item.added / retrieved
{
  "type": "conversation.item.added",
  "event_id": "event_C9G8pjSJCfRNEhMEnYAVy",
  "items": [
    {
      "id": "item_C9G8pGVKYnaZu8PH5YQ9O",
      "type": "message",
      "status": "completed",
      "role": "user",
      "call_id": "",
      "content": [
        { "type": "input_text", "text": "hi" }
      ]
    }
  ]
}
```

上下文要求（来自接入必读）：

- 上下文须按 `user` / `assistant` **成对提交**，数组长度为偶数。
- 时间戳需统一：要么全部携带（严格递增、不超过当前时间），要么全部不携带。
- 接续历史对话：复用上次 `session.created` 返回的 `session.id`（即 `dialog.id`）即可接续，服务端默认保留最近 **20 轮问答记录**。

### 17.2 更新上下文 conversation.item.update

```json
{
  "event_id": "event_901",
  "type": "conversation.item.update",
  "items": [
    {
      "id": "item_002",
      "content": [
        { "type": "input_text", "text": "更新后的文本" }
      ]
    }
  ]
}
```

### 17.3 查询上下文 conversation.item.retrieve

```json
{
  "event_id": "event_901",
  "type": "conversation.item.retrieve",
  "items": [
    { "id": "item_002" }
  ]
}
```

服务端返回 `conversation.item.retrieved`，返回查询到的上下文内容。

### 17.4 删除上下文 conversation.item.delete

```json
{
  "event_id": "event_901",
  "type": "conversation.item.delete",
  "items": [
    { "id": "item_003" }
  ]
}
```

服务端返回：

```json
// conversation.item.deleted
{
  "event_id": "event_2526",
  "type": "conversation.item.deleted"
}
```

若没有可删除的上下文，可能返回：

```json
{
  "status_code": 40000010,
  "message": "empty conversation deleted messages"
}
```

---

## 18. Function Calling

### 18.1 声明工具

在 `session.create` / `session.update` 的 `tools` 字段中以标准 JSON Schema 结构声明（`type = function` + `parameters`）。会话过程中可再次发送 `session.update` 动态增删工具。

```json
[
  {
    "type": "function",
    "name": "get_weather",
    "description": "查询指定城市天气",
    "parameters": {
      "type": "object",
      "properties": {
        "city": {
          "type": "string",
          "description": "城市名称"
        }
      },
      "required": ["city"]
    }
  }
]
```

> **注意**：`session.update` 动态增删工具为**全量覆盖**，非增量更新。

### 18.2 服务端下发函数调用

当模型决定调用工具时，服务端返回 `response.function_call_arguments.done`：

```json
{
  "event_id": "event_8899",
  "type": "response.function_call_arguments.done",
  "items": [
    {
      "id": "item_001",
      "type": "function_call",
      "call_id": "call_weather_001",
      "name": "get_weather",
      "arguments": "{\"city\":\"Beijing\"}"
    },
    {
      "id": "item_002",
      "type": "function_call",
      "call_id": "call_stock_002",
      "name": "get_stock",
      "arguments": "{\"stock_code\":\"600036\",\"market\":\"sh\"}"
    }
  ]
}
```

| 字段 | 说明 |
| --- | --- |
| `call_id` | 本次函数调用的唯一标识，**回传结果时必须一致**。 |
| `name` | 函数名，与 `tools` 中声明的 `name` 对应。 |
| `arguments` | 模型生成的参数，JSON 字符串。客户端需 `JSON.parse` 后执行本地函数。 |

### 18.3 回传函数结果

客户端执行本地函数后，须通过 `conversation.item.create` 回传结果（`role=tool`，结果放入 `content` 数组），并在回传项中携带**相同的 `call_id`**：

```json
{
  "event_id": "client_event_1234",
  "type": "conversation.item.create",
  "items": [
    {
      "call_id": "call_weather_001",
      "role": "tool",
      "content": [
        {
          "type": "input_text",
          "text": "今天北京天气晴，气温22~30℃，微风"
        }
      ]
    },
    {
      "call_id": "call_stock_002",
      "role": "tool",
      "content": [
        {
          "type": "input_text",
          "text": "当前股价33.25元，涨幅+1.23%"
        }
      ]
    }
  ]
}
```

> **注意**：
> - `call_id` 不一致会导致结果无法匹配、回复异常。
> - **并行调用**：单次 `response.function_call_arguments.done` 的 `items` 可能包含多个函数调用，需按每个 `call_id` 分别执行，并将结果**聚合后一次性回传**；全部回传完成后，模型才会继续生成最终回复。
> - 函数执行结果建议序列化为 JSON 字符串后放入 `content[].text`，结构清晰，便于模型解析。

---

## 19. 下行事件总表

| 类别 | 事件名 | 说明 |
| --- | --- | --- |
| Session | `session.created` | 会话已成功启动，返回 `session.id`（对应原版本 `dialog.id`），可用于接续历史对话内容。 |
| Session | `session.updated` | `session.update` 请求对应的确认响应（ack），表示会话配置已成功更新。 |
| Session | `session.closed` | 会话已结束。 |
| Audio | `input_audio_buffer.committed` | 输入音频缓冲区已成功提交，标志着一次用户音频输入的结束。 |
| ASR | `conversation.item.input_audio_transcription.started` | 模型识别出音频流中的首字时返回。 |
| ASR | `conversation.item.input_audio_transcription.delta` | 模型实时识别出的用户说话文本内容（流式增量）。 |
| ASR | `conversation.item.input_audio_transcription.completed` | 模型判定用户说话结束时返回。 |
| ASR | `conversation.item.input_audio_transcription.failed` | ASR 识别失败。 |
| Chat | `response.output_text.delta` | 模型回复的文本内容（流式增量）。 |
| Chat | `response.output_text.done` | 模型回复文本生成结束。 |
| TTS | `response.output_audio.started` | 合成音频的起始事件。 |
| TTS | `response.output_audio.delta` | 返回的流式音频数据块，Base64 编码。 |
| TTS | `response.output_audio.done` | 模型一轮音频合成结束。 |
| Context | `conversation.item.added` | 新增上下文请求的确认（ack），返回创建成功的上下文数组。 |
| Context | `conversation.item.retrieved` | 查询上下文请求的确认（ack），返回查询到的上下文内容。 |
| Context | `conversation.item.deleted` | 删除上下文请求的确认（ack），返回被删除的上下文内容。 |
| FC | `response.function_call_arguments.done` | FC 函数调用参数生成完成。 |
| Usage | `response.done` | 一轮交互结束，返回本次用量统计。 |
| Usage | `response.canceled` | 客户端打断请求 `response.cancel` 的确认（ack）。 |
| Error | `error` | 错误事件，错误表详见 [21. 错误码](#21-错误码)。 |

---

## 20. 下行事件示例

以下是"发送音频流"场景下服务端返回的完整响应链路示例。

### 20.1 ASR 识别开始

```json
// started（ASRInfo）
{
  "type": "conversation.item.input_audio_transcription.started",
  "event_id": "event_0",
  "item_id": ""
}
```

### 20.2 ASR 识别增量

```json
// delta（ASRResponse）
{
  "type": "conversation.item.input_audio_transcription.delta",
  "event_id": "event_CCXGRxsAimPAs8kS2Wc7Z",
  "item_id": "item_CCXGQ4e1ht4cOraEYcuR2",
  "content_index": 0,
  "delta": "Hey"
}
```

### 20.3 ASR 识别完成

```json
// completed（ASREnded）
{
  "type": "conversation.item.input_audio_transcription.completed",
  "event_id": "event_CCXGRvtUVrax5SJAnNOWZ",
  "item_id": "item_CCXGQ4e1ht4cOraEYcuR2",
  "content_index": 0,
  "transcript": "Hey, can you hear me?"
}
```

### 20.4 ASR 识别失败

```json
// failed（ASRFailed）
{
  "event_id": "event_2324",
  "type": "conversation.item.input_audio_transcription.failed",
  "item_id": "msg_003",
  "error": {
    "type": "transcription_error",
    "code": "audio_unintelligible",
    "message": "The audio could not be transcribed.",
    "param": null
  }
}
```

### 20.5 模型回复文本增量

```json
// response.output_text.delta
{
  "event_id": "event_4142",
  "type": "response.output_text.delta",
  "question_id": "question_001",
  "response_id": "resp_001",
  "delta": "Sure, I can h"
}
```

### 20.6 模型回复文本完成

```json
// response.output_text.done
{
  "event_id": "event_4344",
  "type": "response.output_text.done",
  "question_id": "question_001",
  "response_id": "resp_001",
  "text": "Sure, I can help with that."
}
```

### 20.7 音频合成开始

```json
// response.output_audio.started
{
  "event_id": "event_4",
  "type": "response.output_audio.started",
  "question_id": "question_001",
  "response_id": "",
  "tts_type": ""
}
```

### 20.8 音频合成增量

```json
// response.output_audio.delta
{
  "event_id": "event_4950",
  "type": "response.output_audio.delta",
  "question_id": "question_001",
  "response_id": "resp_001",
  "delta": "Base64EncodedAudioDelta"
}
```

### 20.9 音频合成结束

```json
// response.output_audio.done
{
  "event_id": "event_5152",
  "type": "response.output_audio.done",
  "question_id": "question_001",
  "response_id": "resp_001",
  "status_code": "20000002"
}
```

---

## 21. 错误码

客户端应对所有下行 `error` 事件做统一捕获与日志记录。推荐在每条上行事件中携带 `event_id`，以便结合服务端返回的 `X-Tt-Logid` 进行精准的问题定位与链路追踪。

| 错误码 | 关键字 | 描述与处理建议 |
| --- | --- | --- |
| `42000020` | `volc_websearch_bot_id is required` | `web_agent` 联网模式缺 `bot_id`。 |
| `42000020` | `volc_websearch_api_key is required` | 联网模式缺少 `api_key`。 |
| `45000003` | `Abnormal silence audio` | 超过 10 分钟无交互，服务端释放连接。 |
| `50000000` | `AudioQueryError` | 模型推理出错。 |
| `50000000` | `found unknown escape character` | `speaking_style` / `system_role` 含非法字符，建议检查 prompt。 |
| `55000001` | `ServerError` | 模型推理出错。 |
| `55000001` | `ContextCanceled` | 未正常发送 `session.close` 即断开，务必收到回复再断连。 |
| `55000001` | `ClientError:InvalidSpeaker` | 音色名不合法。 |
| `55000001` | `ExceededConcurrentDurationLimit` | 并发时长超限。 |
| `50700000` | `stream recv timeout` | 模型推理超时。 |
| `52000022` | `AudioChatError` | 模型推理出错。 |
| `52000035` | `S2SQueryConnectError` | 模型推理出错。 |

**处理策略**：

- 遇服务端 **5xx 错误** 统一触发重连。
- 遇**鉴权 / 参数类错误（4xx）** 应停止重试并核对配置。
- 每条上行事件建议携带 `event_id`，结合响应头 `X-Tt-Logid` 排障。

---

## 22. 稳定性与限流

- **优雅关闭**：务必先发送 `session.close`（即 `FinishSession`）并收到服务端回复后，再关闭 WebSocket 链接；直接断开会触发 `ContextCanceled` 错误（错误码 `55000001`）。
- **5xx 重连**：遇到服务端 5xx 错误时，应统一触发重连逻辑。
- **限流说明**：限流维度包括：
  - **QPM**（每分钟 `StartSession` 调用次数，默认 `60`）
  - **TPM**（每分钟 token 消耗量，默认 `100000`）
  - 按 **AppID / 项目维度** 计算。
- 若预估用量将超过配额，请提前联系对应商务沟通扩容。

---

## 23. 接口迁移与事件对照表

若已完成端到端实时语音识别的接入，现需升级至全双工版本，事件名对照如下。

### 23.1 上行事件对照

| 新事件 | 原 S2S 版本事件名 |
| --- | --- |
| `session.create`（首次） | `StartSession` |
| `session.update`（会话中） | `UpdateConfig` |
| `session.close` | `FinishSession` |
| `input_audio_buffer.append` | `TaskRequest` |
| `input_audio_buffer.commit` | `EndASR` |
| `speech_text_buffer.commit` | `SayHello` |
| `speech_text_buffer.replacement.append` / `commit` | `ChatTTSText` |
| `conversation.item.create` | `ConversationCreate` / `FunctionCallOutput` |
| `conversation.item.update` | `ConversationUpdate` |
| `conversation.item.retrieve` | `ConversationRetrieve` |
| `conversation.item.delete` | `ConversationDelete` |
| `response.cancel` | `ClientInterrupt` |

### 23.2 下行事件对照

| 新事件 | 原 S2S 版本事件 |
| --- | --- |
| `session.created` | `SessionStarted` |
| `session.updated` | `SessionUpdated` / `ConfigUpdated` |
| `session.closed` | `SessionFinished` |
| `input_audio_buffer.committed` | `ClientStopped` |
| `conversation.item.input_audio_transcription.started` | `ASRInfo` |
| `conversation.item.input_audio_transcription.delta` | `ASRResponse` |
| `conversation.item.input_audio_transcription.completed` | `ASREnded` |
| `conversation.item.input_audio_transcription.failed` | `ASRFailed` |
| `response.output_text.delta` | `ChatResponse` |
| `response.output_text.done` | `ChatEnded` |
| `response.output_audio.started` | `TTSSentenceStart` |
| `response.output_audio.delta` | `TTSResponse` |
| `response.output_audio.done` | `TTSEnded` |
| `conversation.item.added` | `ConversationCreated` |
| `conversation.item.retrieved` | `ConversationRetrieved` |
| `conversation.item.deleted` | `ConversationDeleted` |
| `response.function_call_arguments.done` | `FunctionCallResponse` |
| `response.done` | `UsageResponse` |
| `error` | `ResponseMeta` / `DialogCommonError` |

### 23.3 迁移最佳实践

- **事件改名优先，配置后置**：建议先将客户端收发的事件名从实时语音版本切换为全双工版本，再将 `asr` / `tts` / `dialog` 配置迁入 `extension`，可显著降低迁移风险。
- **extension 直接复用存量配置**：原 `StartSessionPayload` 中的 `asr` / `tts` / `dialog` 字段无需改造，按原格式整体移入 `extension` 即可。
- **携带 event_id**：推荐所有上行事件在 optional 字段携带 `event_id` 与 `session id`，将状态管理复杂度交由服务端处理，便于链路追踪与排障。
- **model 固定单值**：`session.model` 固定填写 `1.2.6.1`（全双工版本），无需再在 `extension.dialog.extra` 配置 model 选路。

---

## 24. 视频帧采集最佳实践（Doubao-Seed-Realtime-1.0）

> 本节仅针对 `Doubao-Seed-Realtime-1.0` 多模态模型。

**推荐基线**：采集 720p（短边 720：竖屏 720×1280，横屏 1280×720）、JPEG 编码、质量 75、帧率不超过 2 FPS。该档位在视觉理解精度、带宽与服务端处理时延之间性价比最高。

| 维度 | 推荐值 | 说明 |
| --- | --- | --- |
| 分辨率 | 短边 720（竖屏 720×1280 / 横屏 1280×720） | 服务端视觉模型在 720p 下对人脸、屏幕文字、物体识别已足够；不要直接上 1080p / 4K，单帧体积数倍增长但识别增益有限。摄像头原生分辨率过高时客户端先缩放到 720p 再编码。 |
| 编码格式 | JPEG，质量 70–80 | 同画质下 JPEG 比 PNG 小 5–10 倍，强烈推荐；只有当画面包含大量文字、需要无损边缘时才用 PNG。 |
| 帧率 | ≤ 2 FPS（建议 500 ms / 帧） | 服务端限流上限为 2 FPS，客户端不可调高；超过 2 FPS 的帧会被网关丢弃，不要浪费带宽。 |
| 单帧大小 | ≤ 约 100 KB（Base64 后约 130 KB） | 720p JPEG q75 典型在 60–100 KB；超过时优先降质量（q65–70）而不是降分辨率。 |
| 上行带宽 | ≈ 200 KB/s ≈ 1.6 Mbps | 2 FPS × 100 KB，叠加 16 kHz 音频（约 25 KB/s）后总上行约 1.8 Mbps，弱网降级时优先保音频、视频帧再降。 |
| 动态降帧 | 静止场景 0.5–1 FPS，有动作 / 屏幕内容变化时回到 2 FPS | 客户端做帧间差异检测，画面长时间静止时主动降帧；检测到人脸移动、屏幕翻页等事件时立即补一帧。 |
| 屏幕共享 | 裁剪到内容主区域，避免全屏 1080p 录制 | 共享窗口时聚焦当前窗口内容，缩放到 720p；多屏场景只共享当前激活屏。 |
| 隐私 | 敏感区域按业务要求打码 | 会议场景下对人脸、文档中的隐私信息（工牌、账号、合同金额等）按合规要求做模糊 / 马赛克后再上行。 |
| 方向 | 跟随采集源原生方向 | 不要在客户端做旋转 / 镜像，避免额外延迟；需要时让服务端按元数据处理。 |

---

## 25. 推荐接入流程

1. 在控制台获取 API Key（控制台 > API Key 管理）。
2. 建立到 `wss://openspeech.bytedance.com/api/v3/duplex/realtime/dialogue` 的 WebSocket 连接，并设置 `X-Api-Key` 请求头。
3. 发送 `session.create`，配置模型（`1.2.6.1`）、输入输出音频格式、音色、系统提示词和可选 `extension`。
4. 等待 `session.created`，保存 `session.id`。
5. 若需要开场白，发送 `speech_text_buffer.commit`（第 13 节）。
6. 开始采集麦克风音频，按 20 ms 一包（640 字节）发送 `input_audio_buffer.append`（第 10 节）。
7. 监听下行事件：ASR 增量 / 完成、模型回复文本、TTS 音频增量、函数调用、用量统计、错误。
8. 收到 `response.output_audio.delta` 时，解码 Base64 并写入播放队列。
9. 麦克风关闭时发送 `input_audio_mute.commit`，恢复时发送 `input_audio_unmute.commit` 保活（第 12 节）。
10. 遇到函数调用，执行本地函数并用 `conversation.item.create`（`role=tool`）回传结果，携带相同 `call_id`（第 18 节）。
11. 用户抢话或需要中止回复时发送 `response.cancel`（第 15 节）。
12. 需要手动结束输入时发送 `input_audio_buffer.commit`（第 11 节）。
13. 需要由客户端指定回复话术时发送 `speech_text_buffer.replacement.append` / `commit`（第 14 节）。
14. 会话结束时发送 `session.close`，等待 `session.closed` 后再断开 WebSocket（第 16 节）。

**最简交互时序**（来自接入必读）：

```text
客户端                                   服务端
  │                                        │
  │── session.create ────────────────────►│
  │◄─────────────────── session.created ──│
  │                                        │
  │── input_audio_buffer.append (循环) ─►│
  │◄──── ASR delta / completed ───────────│
  │◄──── response.output_text.delta ──────│
  │◄──── response.output_audio.delta ─────│
  │◄──── response.output_audio.done ──────│
  │◄──── response.done ───────────────────│
  │                                        │
  │── session.close ─────────────────────►│
  │◄─────────────────── session.closed ───│
  │                                        │
```

---

## 26. 最小实现示例（Python）

以下示例使用 Python `websockets` 库，仅演示连接、创建会话和发送音频事件的核心流程。

```python
import asyncio
import base64
import json
import uuid

import websockets

API_KEY = "your_api_key"
URL = "wss://openspeech.bytedance.com/api/v3/duplex/realtime/dialogue"


async def main():
    headers = {"X-Api-Key": API_KEY}

    async with websockets.connect(URL, extra_headers=headers) as ws:
        create_payload = {
            "type": "session.create",
            "session": {
                "model": "1.2.6.1",
                "instructions": "你是一个简洁友好的中文语音助手。",
                "audio": {
                    "input": {
                        "format": {"type": "pcm", "rate": 16000}
                    },
                    "output": {
                        "format": {"type": "ogg_opus", "rate": 24000},
                        "voice": "zh_female_vv_jupiter_bigtts",
                    },
                },
            },
        }
        await ws.send(json.dumps(create_payload))

        async for message in ws:
            event = json.loads(message)
            print("down:", event["type"])

            if event["type"] == "session.created":
                session_id = event["session"]["id"]
                print("session id:", session_id)

                # 这里应替换为真实采集到的 20ms PCM 分片（640 字节）。
                pcm_chunk = b"\x00" * 640
                audio_event = {
                    "type": "input_audio_buffer.append",
                    "event_id": uuid.uuid4().hex,
                    "audio": base64.b64encode(pcm_chunk).decode("ascii"),
                }
                await ws.send(json.dumps(audio_event))

                stop_event = {
                    "type": "input_audio_buffer.commit",
                    "event_id": uuid.uuid4().hex,
                }
                await ws.send(json.dumps(stop_event))

            if event["type"] == "response.output_audio.delta":
                audio = base64.b64decode(event["delta"])
                # 实际应用中应将 audio 写入播放队列。
                print("audio bytes:", len(audio))

            if event["type"] in ("error", "session.closed"):
                break


asyncio.run(main())
```

---

## 27. 开发注意事项

- **鉴权**：使用请求头 `X-Api-Key`；浏览器端无法自定义 WebSocket 请求头，须通过后端代理注入，避免泄露 API Key。
- **音频节奏**：严格按实时音频流节奏发送 `input_audio_buffer.append`，过快或过慢都会触发服务端错误；20 ms 一包（640 字节）。
- **保活**：关闭麦克风后必须发送 `input_audio_mute.commit`，否则服务端会因收不到音频触发超时导致无响应。
- **工具更新**：`session.update` 的 `tools` 是**全量覆盖**，不是增量更新。
- **Function Calling**：回传结果时 `call_id` 必须与下发一致；并行调用需分别执行后聚合一次性回传。
- **上下文**：初始化必须 `user` / `assistant` 成对提交；时间戳要么全带（严格递增）要么全不带；服务端默认保留最近 20 轮问答。
- **事件追踪**：所有上行事件建议携带 `event_id`，结合响应头 `X-Tt-Logid` 排障。
- **优雅关闭**：必须先发 `session.close` 并收到 `session.closed` 后再断开 WebSocket，否则触发 `ContextCanceled`（`55000001`）。
- **错误处理**：5xx 统一触发重连；4xx 鉴权 / 参数错误停止重试并核对配置。
- **音色**：使用复刻音色时 `voice` 填复刻音色 ID；开启唱歌能力建议使用官方主推的 Vivi 2.0、小何 2.0、云舟 2.0、小天 2.0 四个音色；方言参数仅这 4 个音色支持。
- **视频（Doubao-Seed-Realtime-1.0）**：720p、JPEG q75、≤ 2 FPS，单帧 ≤ 100 KB，弱网优先保音频。
