#!/usr/bin/env python3
"""Qwen-Audio-Realtime 适配闭环测试（对接 mock_qwen_upstream）。

覆盖：握手地址/鉴权头、首次 session.update 后才就绪、工具结构转换、音区机制 A/B
（B 为回复结束后 create+delete 替换并 retrieve 校验）、FC 写回 + response.create、
历史上下文聚合、打招呼临时 user 指令、成对上下文注入、空闲打断、音色变更提示、主动关闭。
"""
import base64
import json
import os
import socket
import sys
import threading
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.dirname(__file__))

import server  # noqa: E402
from mock_qwen_upstream import MockQwenUpstream  # noqa: E402

PORT = 8911
SILENCE = base64.b64encode(b"\x00" * 640).decode("ascii")

server.MinimalWebSocketClient._open_tls_socket = (
    lambda self, verify: socket.create_connection(("127.0.0.1", PORT), timeout=20)
)


def drain_events(session, collected, stop):
  while not stop.is_set():
    try:
      collected.append(session.events.get(timeout=0.2))
    except Exception:
      continue


def wait_for(predicate, timeout=8.0, what=""):
  deadline = time.time() + timeout
  while time.time() < deadline:
    result = predicate()
    if result:
      return result
    time.sleep(0.05)
  raise AssertionError(f"timeout waiting for {what}")


def sent(mock, mtype):
  with mock.lock:
    return [m for m in mock.received_messages if m.get("type") == mtype]


def local(collected, etype):
  return [ev for ev in list(collected) if ev.get("type") == etype]


def speak_turn(session):
  for _ in range(15):
    session.send_audio(SILENCE)
    time.sleep(0.005)


def main():
  mock = MockQwenUpstream(port=PORT)
  threading.Thread(target=mock.serve, daemon=True).start()
  time.sleep(0.3)

  config = {
      "provider": "qwen",
      "api_key": "sk-test",
      "tool_root": str(server.default_tool_root()),
      "enable_tools": True,
      "tools": None,
      "instructions": "你是座舱助手。当前说话音区：{current_speaking_zone}",
      "zone": "主驾",
      "enable_zone_history_labels": True,
      "voice": "",
      "speech_rate": 1.2,
      "location": {"city": "杭州市"},
      **server.normalize_qwen_create_config({}),
  }
  session = server.STATE.create(config)
  assert isinstance(session, server.QwenRealtimeSession)
  collected, stop = [], threading.Event()
  threading.Thread(target=drain_events, args=(session, collected, stop), daemon=True).start()

  # 1. 握手与就绪
  wait_for(lambda: local(collected, "local.ready"), what="local.ready")
  assert mock.request_line.startswith("GET /api-ws/v1/realtime?model=qwen-audio-3.1-realtime-plus "), mock.request_line
  assert mock.request_headers.get("authorization") == "Bearer sk-test"
  assert mock.request_headers.get("host") == "dashscope.aliyuncs.com"
  first = sent(mock, "session.update")[0]["session"]
  assert first["voice"] == "longanqian_v3.1"
  assert first["turn_detection"] == {"type": "smart_turn_v2"}
  assert first["speech_rate"] == 1.2
  assert "当前说话音区：主驾" in first["instructions"] and "城市：杭州市" in first["instructions"]
  assert all("function" in tool and "name" in tool["function"] for tool in first["tools"]), first["tools"]
  assert local(collected, "local.session_create")[0]["title"] == "session.update（首次）"
  print("== handshake / first session.update / ready OK")

  # 2. 普通一轮 + 音区机制 B（create+delete 替换）
  speak_turn(session)
  acked = wait_for(lambda: [e for e in local(collected, "local.zone_stamp") if e.get("status") == "acked"],
                   what="zone stamp acked")
  assert acked[0]["replaced_item_id"] == "item_u1", acked
  roles_texts = mock.context_texts()
  assert roles_texts[0] == ("user", "【主驾】打开空调1"), roles_texts
  assert roles_texts[1][0] == "assistant", roles_texts
  assert "item_u1" not in session.known_item_ids and acked[0]["item_id"] in session.known_item_ids
  print("== zone stamp B via create+delete OK")

  # 3. 音区机制 A
  assert session.set_zone("副驾") is True
  wait_for(lambda: len(sent(mock, "session.update")) >= 2, what="zone session.update")
  update = sent(mock, "session.update")[-1]["session"]
  assert "当前说话音区：副驾" in update["instructions"] and "voice" not in update and "turn_detection" not in update
  print("== zone A session.update OK")

  # 4. FC：写回 function_call_output + response.create，二轮结束后才盖章
  mock.fc_next_turn = True
  speak_turn(session)
  wait_for(lambda: len([e for e in local(collected, "local.zone_stamp") if e.get("status") == "acked"]) >= 2,
           what="second zone stamp acked")
  tool_results = local(collected, "local.tool_result")
  assert tool_results and tool_results[0]["result"]["ok"] is True
  outputs = [m for m in sent(mock, "conversation.item.create") if (m.get("item") or {}).get("type") == "function_call_output"]
  assert outputs and outputs[0]["item"]["call_id"] == "call_1"
  with mock.lock:
    types = list(mock.received_types)
  fc_index = types.index("conversation.item.create", types.index("session.update", 1))
  assert "response.create" in types[fc_index:], types
  texts = [text for role, text in mock.context_texts() if role == "user"]
  assert texts == ["【主驾】打开空调1", "【副驾】打开空调2"], texts
  print("== function call round trip + zone stamp after 2nd response OK")

  # 5. 历史上下文聚合
  mark = len(collected)
  session.retrieve_context()
  ctx = wait_for(lambda: [e for e in collected[mark:] if e.get("type") == "local.context_retrieved"], what="context")
  assert not ctx[0]["missing"], ctx[0]
  assert [item["id"] for item in ctx[0]["items"]] == mock.order, (ctx[0]["items"], mock.order)
  outgoing = [e for e in collected[mark:] if e.get("type") == "local.outgoing"]
  assert len(outgoing) == 1 and outgoing[0]["event"].get("item_ids"), outgoing
  print("== context retrieve aggregation OK")

  # 6. 打招呼：临时 user 指令 + response.create，回复结束后删除
  session.send_text("你好呀")
  greet_id = wait_for(lambda: [m["item"]["id"] for m in sent(mock, "conversation.item.create")
                               if str(m["item"].get("id") or "").startswith("item_greet_")], what="greet item")[0]
  wait_for(lambda: any(m.get("item_id") == greet_id for m in sent(mock, "conversation.item.delete")), what="greet delete")
  assert greet_id not in mock.items and greet_id not in session.known_item_ids
  print("== greeting via temporary user item OK")

  # 7. 成对上下文注入：逐条 item.create，assistant 用 output_text
  before = len(sent(mock, "conversation.item.create"))
  assert session.add_context_pairs([{"user": "导航回家", "assistant": "好的"}]) == 2
  wait_for(lambda: len(sent(mock, "conversation.item.create")) >= before + 2, what="context add")
  user_item, assistant_item = [m["item"] for m in sent(mock, "conversation.item.create")[before:before + 2]]
  assert user_item["content"][0]["text"] == "【副驾】导航回家"
  assert assistant_item["content"][0]["type"] == "output_text"
  print("== context pairs OK")

  # 7b. 文本对话：空闲时 item.create(user, 带音区前缀) + response.create，消息保留在上下文
  before = len(sent(mock, "conversation.item.create"))
  replies = len(sent(mock, "response.create"))
  assert session.send_user_text("讲个笑话") == "【副驾】讲个笑话"
  wait_for(lambda: len(sent(mock, "response.create")) > replies, what="text chat response.create")
  item = sent(mock, "conversation.item.create")[before]["item"]
  assert item["role"] == "user" and item["content"][0]["text"] == "【副驾】讲个笑话", item
  assert ("user", "【副驾】讲个笑话") in mock.context_texts()

  update_count = len(sent(mock, "session.update"))
  session.update_runtime_config(session.parse_runtime_update({
      "voice": session._voice(),
      "instructions": session.config["instructions"],
      "enable_zone_history_labels": False,
  }))
  wait_for(lambda: len(sent(mock, "session.update")) > update_count, what="history label session.update")
  updated_instructions = sent(mock, "session.update")[-1]["session"]["instructions"]
  assert server.ZONE_HISTORY_LABEL_INSTRUCTION not in updated_instructions
  before = len(sent(mock, "conversation.item.create"))
  assert session.send_user_text("不加音区前缀") == "不加音区前缀"
  wait_for(lambda: len(sent(mock, "conversation.item.create")) > before, what="unlabeled text item")
  assert sent(mock, "conversation.item.create")[before]["item"]["content"][0]["text"] == "不加音区前缀"

  # 回复进行中：先 response.cancel，response.done 之后才补发 response.create
  session.response_active = True
  replies = len(sent(mock, "response.create"))
  session.send_user_text("换个话题")
  wait_for(lambda: sent(mock, "response.cancel"), what="text chat cancel")
  time.sleep(0.2)
  assert len(sent(mock, "response.create")) == replies, "response.create must wait for response.done"
  session._handle_response_done({"type": "response.done", "response": {"status": "cancelled"}})
  wait_for(lambda: len(sent(mock, "response.create")) > replies, what="deferred response.create")
  with mock.lock:
    types = list(mock.received_types)
  tail = types[types.index("response.cancel"):]
  assert tail[:3] == ["response.cancel", "conversation.item.create", "response.create"], tail
  print("== text chat (idle + barge-in) OK")

  # 8. 空闲时打断不下发 response.cancel；音色变更提示需重连
  cancels = len(sent(mock, "response.cancel"))
  session.cancel_response()
  assert len(sent(mock, "response.cancel")) == cancels
  wait_for(lambda: any("没有进行中的推理" in e.get("message", "") for e in local(collected, "local.warning")),
           what="cancel warning")
  session.update_runtime_config(session.parse_runtime_update({"voice": "longanlingxin", "speech_rate": 1.5}))
  wait_for(lambda: any("重新连接" in e.get("message", "") for e in local(collected, "local.warning")),
           what="voice warning")
  wait_for(lambda: sent(mock, "session.update")[-1]["session"].get("speech_rate") == 1.5, what="speech_rate update")
  print("== cancel guard / voice warning OK")

  # 9. 主动关闭：只有 local.closed，没有 local.error
  session.close()
  wait_for(lambda: local(collected, "local.closed"), what="local.closed")
  assert not local(collected, "local.error"), local(collected, "local.error")
  print("== close OK")

  stop.set()
  check_idle_timeout()
  print("\nALL ASSERTIONS PASSED")


def check_idle_timeout():
  """上游 180 秒无回复主动关闭：应下发 local.session_expired 并正常收尾，不产生 local.error。"""
  port = PORT + 1
  server.MinimalWebSocketClient._open_tls_socket = (
      lambda self, verify: socket.create_connection(("127.0.0.1", port), timeout=20)
  )
  mock = MockQwenUpstream(port=port)
  threading.Thread(target=mock.serve, daemon=True).start()
  time.sleep(0.3)
  session = server.STATE.create({
      "provider": "qwen", "api_key": "sk-test", "tool_root": str(server.default_tool_root()),
      "enable_tools": True, "tools": None, "instructions": "", "zone": "", **server.normalize_qwen_create_config({}),
  })
  collected, stop = [], threading.Event()
  threading.Thread(target=drain_events, args=(session, collected, stop), daemon=True).start()
  wait_for(lambda: local(collected, "local.ready"), what="idle session ready")
  mock.send_idle_timeout()
  wait_for(lambda: local(collected, "local.closed"), what="idle local.closed")
  assert local(collected, "local.session_expired"), "missing local.session_expired"
  assert not local(collected, "local.error"), local(collected, "local.error")
  stop.set()
  print("== idle timeout handled as normal end OK")


if __name__ == "__main__":
  main()
