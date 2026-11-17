#!/usr/bin/env python3
"""Mock Qwen-Audio-Realtime upstream：按 docs/qwen-audio-realtime-integration.md 的时序脚本化回放。

复用 MockUpstream 的 WS 帧收发，覆盖握手（记录请求行与请求头）和事件处理：
- 握手后服务端先发 session.created；session.update → session.updated
- append 累计到阈值触发一轮 smart_turn：speech_started → speech_stopped → transcription.completed
  → committed → conversation.item.created(user) → 一轮回复（或 function_call）
- conversation.item.create/delete/retrieve 维护有序上下文链表，重复 id 返回 invalid_request_error
- response.create 触发回复；response.cancel 在无进行中推理时返回 invalid_request_error
"""
import json
import struct
import threading

from mock_upstream import MockUpstream, ws_accept_key


class MockQwenUpstream(MockUpstream):
  def __init__(self, host="127.0.0.1", port=8911):
    super().__init__(host=host, port=port)
    self.request_line = ""
    self.request_headers = {}
    self.items = {}  # item_id -> item，按插入顺序维护链表
    self.order = []
    self.fc_next_turn = False
    self.turn_seq = 0
    self.resp_seq = 0
    self.speech_started = False

  # ---- 握手：服务端先下发 session.created ----

  def _handshake(self, conn):
    data = b""
    while b"\r\n\r\n" not in data:
      data += conn.recv(4096)
    head, _, rest = data.partition(b"\r\n\r\n")
    lines = head.decode("latin-1").split("\r\n")
    self.request_line = lines[0]
    for line in lines[1:]:
      if ":" in line:
        key, value = line.split(":", 1)
        self.request_headers[key.strip().lower()] = value.strip()
    resp = (
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        f"Sec-WebSocket-Accept: {ws_accept_key(self.request_headers.get('sec-websocket-key', ''))}\r\n\r\n"
    )
    if rest:
      self._buf = rest
    # 101 响应与 session.created 合并为一次写出：真实上游握手后立即推首个事件，
    # 客户端必须保留与响应头同包到达的帧字节。
    created = json.dumps({"type": "session.created", "event_id": "ev_created",
                          "session": {"id": "sess_mock", "object": "realtime.session"}}).encode("utf-8")
    conn.sendall(resp.encode("ascii") + bytes([0x81, 126]) + struct.pack("!H", len(created)) + created)
    self.note("up->client", "101 + session.created (single write)")

  def send_idle_timeout(self):
    """测试钩子：模拟上游 180 秒无回复的空闲超时——下发 error 后以 1007 关闭连接。"""
    reason = "Your session was closed because no response was generated for 180 seconds."
    self._send_json(self.conn, {"type": "error", "event_id": "ev_idle", "error": {
        "type": "invalid_request_error", "code": "response_idle_timeout", "message": reason}})
    payload = struct.pack("!H", 1007) + reason.encode("utf-8")
    self.conn.sendall(bytes([0x88, len(payload)]) + payload)

  # ---- 上下文链表 ----

  def _insert_item(self, item, previous_item_id=None):
    item_id = item["id"]
    self.items[item_id] = item
    if previous_item_id in self.order:
      self.order.insert(self.order.index(previous_item_id) + 1, item_id)
    else:
      self.order.append(item_id)
    index = self.order.index(item_id)
    return self.order[index - 1] if index else None

  def context_texts(self):
    out = []
    for item_id in self.order:
      item = self.items[item_id]
      text = "".join(str(part.get("text") or part.get("transcript") or "") for part in item.get("content") or [])
      out.append((item.get("role") or item.get("type"), text or item.get("output") or item.get("arguments") or ""))
    return out

  def _error(self, conn, message, param=""):
    self._send_json(conn, {"type": "error", "event_id": "ev_err", "error": {
        "type": "invalid_request_error", "code": "invalid_value", "message": message, "param": param}})

  # ---- 事件处理 ----

  def _on_message(self, conn, raw):
    msg = json.loads(raw)
    mtype = msg.get("type")
    with self.lock:
      self.received_types.append(mtype)
      self.received_messages.append(msg)
    if mtype != "input_audio_buffer.append":
      self.note("client->up", json.dumps(msg, ensure_ascii=False)[:220])

    if mtype == "session.update":
      self._send_json(conn, {"type": "session.updated", "event_id": "ev_upd", "session": msg.get("session") or {}})
    elif mtype == "input_audio_buffer.append":
      self.frame_count += 1
      self._drive_turn(conn)
    elif mtype == "conversation.item.create":
      item = dict(msg.get("item") or {})
      item.setdefault("id", f"item_c{len(self.items)}")
      if item["id"] in self.items:
        self._error(conn, f"item {item['id']} already exists", "item.id")
        return
      previous = self._insert_item(item, msg.get("previous_item_id"))
      self._send_json(conn, {"type": "conversation.item.created", "event_id": "ev_ic",
                             "previous_item_id": previous, "item": item})
    elif mtype == "conversation.item.delete":
      item_id = msg.get("item_id")
      if item_id not in self.items:
        self._error(conn, f"item {item_id} not found", "item_id")
        return
      self.items.pop(item_id)
      self.order.remove(item_id)
      self._send_json(conn, {"type": "conversation.item.deleted", "event_id": "ev_id", "item_id": item_id})
    elif mtype == "conversation.item.retrieve":
      item = self.items.get(msg.get("item_id"))
      if not item:
        self._error(conn, "item not found", "item_id")
        return
      self._send_json(conn, {"type": "conversation.item.retrieved", "event_id": "ev_ir", "item": item})
    elif mtype == "response.create":
      self._respond(conn, "好的，已经处理完成")
    elif mtype == "response.cancel":
      self._error(conn, "no response in progress", "response.cancel")
    else:
      self.note("mock", f"ignore {mtype}")

  def _drive_turn(self, conn):
    if self.frame_count == 5:
      self.turn_seq += 1
      self.speech_started = True
      self._send_json(conn, {"type": "input_audio_buffer.speech_started", "event_id": "ev_ss",
                             "audio_start_ms": 100, "item_id": f"item_u{self.turn_seq}"})
    if self.frame_count == 15 and self.speech_started:
      self.speech_started = False
      self.frame_count = 0
      item_id = f"item_u{self.turn_seq}"
      transcript = f"打开空调{self.turn_seq}"
      self._send_json(conn, {"type": "input_audio_buffer.speech_stopped", "event_id": "ev_sp",
                             "audio_end_ms": 300, "item_id": item_id})
      self._send_json(conn, {"type": "conversation.item.input_audio_transcription.completed",
                             "event_id": "ev_tc", "item_id": item_id, "content_index": 0, "transcript": transcript})
      item = {"id": item_id, "object": "realtime.item", "type": "message", "role": "user",
              "content": [{"type": "input_audio", "transcript": transcript}]}
      previous = self._insert_item(item)
      self._send_json(conn, {"type": "input_audio_buffer.committed", "event_id": "ev_cm",
                             "previous_item_id": previous, "item_id": item_id})
      self._send_json(conn, {"type": "conversation.item.created", "event_id": "ev_uc",
                             "previous_item_id": previous, "item": item})
      if self.fc_next_turn:
        self.fc_next_turn = False
        self._respond_function_call(conn)
      else:
        self._respond(conn, "好的，已为你打开空调")

  def _respond(self, conn, text):
    self.resp_seq += 1
    resp_id = f"resp_{self.resp_seq}"
    item = {"id": f"item_a{self.resp_seq}", "object": "realtime.item", "type": "message", "role": "assistant",
            "status": "completed", "content": [{"type": "audio", "transcript": text}]}
    self._send_json(conn, {"type": "response.created", "event_id": "ev_rc",
                           "response": {"id": resp_id, "status": "in_progress", "output": []}})
    previous = self._insert_item(item)
    self._send_json(conn, {"type": "conversation.item.created", "event_id": "ev_ac",
                           "previous_item_id": previous, "item": item})
    self._send_json(conn, {"type": "response.audio_transcript.delta", "event_id": "ev_td",
                           "response_id": resp_id, "delta": text})
    self._send_json(conn, {"type": "response.audio.delta", "event_id": "ev_ad",
                           "response_id": resp_id, "delta": "AAAAAA=="})
    self._send_json(conn, {"type": "response.audio_transcript.done", "event_id": "ev_tdn",
                           "response_id": resp_id, "transcript": text})
    self._send_json(conn, {"type": "response.audio.done", "event_id": "ev_adn", "response_id": resp_id})
    self._send_json(conn, {"type": "response.done", "event_id": "ev_rd", "response": {
        "id": resp_id, "status": "completed", "output": [item],
        "usage": {"total_tokens": 10, "input_tokens": 6, "output_tokens": 4}}})

  def _respond_function_call(self, conn):
    self.resp_seq += 1
    resp_id = f"resp_{self.resp_seq}"
    args = json.dumps({"component": "air_conditioner", "action": "on"})
    item = {"id": f"item_f{self.resp_seq}", "object": "realtime.item", "type": "function_call",
            "status": "completed", "call_id": "call_1", "name": "vehicle_control", "arguments": args}
    self._send_json(conn, {"type": "response.created", "event_id": "ev_rc",
                           "response": {"id": resp_id, "status": "in_progress", "output": []}})
    previous = self._insert_item(item)
    self._send_json(conn, {"type": "conversation.item.created", "event_id": "ev_fc",
                           "previous_item_id": previous, "item": item})
    self._send_json(conn, {"type": "response.function_call_arguments.done", "event_id": "ev_fad",
                           "response_id": resp_id, "item_id": item["id"], "call_id": "call_1",
                           "name": "vehicle_control", "arguments": args})
    self._send_json(conn, {"type": "response.done", "event_id": "ev_rd", "response": {
        "id": resp_id, "status": "completed", "output": [item]}})


if __name__ == "__main__":
  MockQwenUpstream().serve()
