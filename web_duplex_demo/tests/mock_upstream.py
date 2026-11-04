#!/usr/bin/env python3
"""Mock seeduplex upstream: plain-TCP WS server with scripted ASR turn flow.

Used by test_zone_flow.py to validate the demo server's wire behavior
around mid-turn session.update (zone switch). Run standalone or via harness.
"""
import base64
import hashlib
import json
import socket
import struct
import threading
import time

GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


def ws_accept_key(key):
    return base64.b64encode(hashdog(key)).decode("ascii")


def hashdog(key):
    return hashlib.sha1((key + GUID).encode("ascii")).digest()


class MockUpstream:
  def __init__(self, host="127.0.0.1", port=8901):
    self.addr = (host, port)
    self.log = []
    self.received_types = []
    self.received_messages = []
    self.lock = threading.Lock()
    self.session_id = "mock_session_1"
    self.frame_count = 0
    self.started_sent = False
    self.completed_sent = False
    self.item_seq = 0
    self._buf = b""
    self.conn = None
    self.hold_session_created = False
    self.session_created_sent = False

  def note(self, direction, text):
    line = f"[{time.time():.3f}] {direction} {text}"
    with self.lock:
      self.log.append(line)
    print(line, flush=True)

  def serve(self):
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(self.addr)
    srv.listen(1)
    self.note("mock", f"listening on {self.addr}")
    conn, _ = srv.accept()
    self.handle(conn)

  def handle(self, conn):
    conn.settimeout(30)
    self.conn = conn
    self._handshake(conn)
    while True:
      try:
        text = self._recv_text(conn)
      except Exception as exc:
        self.note("mock", f"recv stopped: {exc}")
        return
      if text is None:
        continue
      self._on_message(conn, text)

  def _handshake(self, conn):
    data = b""
    while b"\r\n\r\n" not in data:
      data += conn.recv(4096)
    head, _, rest = data.partition(b"\r\n\r\n")
    key = ""
    for line in head.decode("latin-1").split("\r\n"):
      if line.lower().startswith("sec-websocket-key:"):
        key = line.split(":", 1)[1].strip()
    resp = (
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        f"Sec-WebSocket-Accept: {ws_accept_key(key)}\r\n\r\n"
    )
    conn.sendall(resp.encode("ascii"))
    self.note("mock", "handshake ok")
    if rest:
      self._buf = rest

  def _send_session_created(self):
    if self.session_created_sent or not self.conn:
      return
    self.session_created_sent = True
    self._send_json(self.conn, {"type": "session.created", "event_id": "ev_created",
                                "session": {"id": self.session_id}})

  def release_session_created(self):
    """测试钩子：补发被扣住的 session.created。"""
    self._send_session_created()

  def _send_text(self, conn, text):
    data = text.encode("utf-8")
    header = bytearray([0x81])
    if len(data) < 126:
      header.append(len(data))
    elif len(data) < 65536:
      header.append(126)
      header.extend(struct.pack("!H", len(data)))
    else:
      header.append(127)
      header.extend(struct.pack("!Q", len(data)))
    conn.sendall(bytes(header) + data)

  def _send_json(self, conn, payload):
    self._send_text(conn, json.dumps(payload, ensure_ascii=False))
    self.note("up->client", json.dumps(payload, ensure_ascii=False)[:220])

  def _recv_text(self, conn):
    first = self._recv_exact(conn, 2)
    opcode = first[0] & 0x0F
    masked = bool(first[1] & 0x80)
    length = first[1] & 0x7F
    if length == 126:
      length = struct.unpack("!H", self._recv_exact(conn, 2))[0]
    elif length == 127:
      length = struct.unpack("!Q", self._recv_exact(conn, 8))[0]
    mask = self._recv_exact(conn, 4) if masked else b""
    data = self._recv_exact(conn, length)
    if masked:
      data = bytes(b ^ mask[i % 4] for i, b in enumerate(data))
    if opcode == 0x8:
      raise ConnectionError("client closed")
    if opcode != 0x1:
      return None
    return data.decode("utf-8", errors="replace")

  def _recv_exact(self, conn, size):
    while len(self._buf) < size:
      chunk = conn.recv(4096)
      if not chunk:
        raise ConnectionError("socket closed")
      self._buf += chunk
    out, self._buf = self._buf[:size], self._buf[size:]
    return out

  def _on_message(self, conn, raw):
    try:
      msg = json.loads(raw)
    except json.JSONDecodeError:
      self.note("client->up", f"unparsed: {raw[:120]}")
      return
    mtype = msg.get("type")
    with self.lock:
      self.received_types.append(mtype)
      self.received_messages.append(msg)
    self.note("client->up", json.dumps(msg, ensure_ascii=False)[:220])

    if mtype == "session.create":
      if self.hold_session_created:
        self.note("mock", "holding session.created (test harness)")
        return
      self._send_session_created()
      return
    if mtype == "session.update":
      self._send_json(conn, {"type": "session.updated", "event_id": msg.get("event_id") or "ev_upd"})
      return
    if mtype == "conversation.item.update":
      self._send_json(conn, {"type": "conversation.item.updated", "event_id": msg.get("event_id") or "ev_iu",
                             "items": msg.get("items") or []})
      return
    if mtype == "conversation.item.retrieve":
      self._send_json(conn, {"type": "conversation.item.retrieved", "event_id": msg.get("event_id") or "ev_ir",
                             "items": []})
      return
    if mtype == "conversation.item.create":
      self._send_json(conn, {"type": "conversation.item.added", "event_id": msg.get("event_id") or "ev_ic",
                             "items": msg.get("items") or []})
      return
    if mtype == "input_audio_buffer.append":
      self.frame_count += 1
      self._drive_turn(conn)
      return
    if mtype == "input_audio_buffer.commit":
      self._send_json(conn, {"type": "input_audio_buffer.committed", "event_id": "ev_commit"})
      self._finish_turn(conn)
      return
    if mtype in ("input_audio_mute.commit", "input_audio_unmute.commit"):
      self.note("mock", f"keep-alive event received: {mtype}")
      return
    if mtype == "session.close":
      self._send_json(conn, {"type": "session.closed", "event_id": "ev_close"})
      return
    self.note("mock", f"ignore {mtype}")

  def _drive_turn(self, conn):
    if self.frame_count >= 5 and not self.started_sent:
      self.started_sent = True
      self.item_seq += 1
      self._send_json(conn, {
          "type": "conversation.item.input_audio_transcription.started",
          "event_id": f"ev_started_{self.item_seq}",
          "item_id": "",
      })
    if self.frame_count >= 12 and self.started_sent and not self.completed_sent:
      pass  # completed is triggered by _finish_turn (server VAD at silence) or commit

  def _finish_turn(self, conn):
    if not self.started_sent or self.completed_sent:
      return
    self.completed_sent = True
    self._send_json(conn, {
        "type": "conversation.item.input_audio_transcription.completed",
        "event_id": f"ev_completed_{self.item_seq}",
        "item_id": f"item_{self.item_seq}",
        "transcript": "现在是谁在跟你说话",
    })
    self._send_json(conn, {"type": "conversation.item.added", "event_id": "ev_added",
                           "items": [{"id": f"item_{self.item_seq}", "type": "message", "role": "user",
                                      "content": [{"type": "input_text", "text": "现在是谁在跟你说话"}]}]})
    self._send_json(conn, {"type": "response.output_text.delta", "event_id": "ev_delta", "delta": "是副驾在说话"})
    self._send_json(conn, {"type": "response.output_text.done", "event_id": "ev_done", "text": "是副驾在说话"})
    self._send_json(conn, {"type": "response.done", "event_id": "ev_respdone"})
    self.started_sent = False
    self.completed_sent = False
    self.frame_count = 0


if __name__ == "__main__":
  MockUpstream().serve()
