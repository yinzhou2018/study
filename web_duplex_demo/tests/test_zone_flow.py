#!/usr/bin/env python3
"""Reproduce the zone-switch flow against a mock upstream.

Scenario: user speaks (audio frames stream), switches zone mid-turn
(between transcription.started and completed), keeps streaming audio,
then the turn completes. Asserts:
  1. our server keeps forwarding audio after session.update,
  2. completed event is received and re-emitted,
  3. reader thread stays alive.
"""
import base64
import json
import os
import socket
import sys
import threading
import time
import uuid

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.dirname(__file__))

import server  # noqa: E402
from mock_upstream import MockUpstream  # noqa: E402

# Route the demo's WSS client to the plain-TCP mock.
server.MinimalWebSocketClient._open_tls_socket = (
    lambda self, verify: socket.create_connection(("127.0.0.1", 8901), timeout=20)
)

SILENCE = base64.b64encode(b"\x00" * 320).decode("ascii")


def drain_events(session, collected, stop):
  while not stop.is_set():
    try:
      ev = session.events.get(timeout=0.2)
    except Exception:
      continue
    collected.append(ev)
    print(f"[sse] {json.dumps(ev, ensure_ascii=False)[:200]}")


def main():
  mock = MockUpstream()
  threading.Thread(target=mock.serve, daemon=True).start()
  time.sleep(0.3)

  config = {
      "api_key": "test-key",
      "tool_root": str(os.path.expanduser("~")),
      "zone": "主驾",
      "enable_tools": False,
      "input_format": "pcm",
  }
  session = server.STATE.create(config)
  collected = []
  stop = threading.Event()
  threading.Thread(target=drain_events, args=(session, collected, stop), daemon=True).start()

  assert session.ready.wait(timeout=10), "session not ready"
  time.sleep(0.2)

  # --- 用户开始说话：流式上行音频，第 5 帧后 mock 发 started ---
  for i in range(7):
    session.send_audio(SILENCE)
    time.sleep(0.02)
  time.sleep(0.3)
  started = [e for e in collected if (e.get("event") or {}).get("type")
             == "conversation.item.input_audio_transcription.started"]
  print(f"\n== started events: {len(started)}")
  assert started, "transcription.started not received"

  # --- 说话中途切换音区（mid-turn session.update） ---
  changed = session.set_zone("副驾")
  print(f"\n== set_zone(副驾) -> {changed}, mid-turn turn_zone stays: {session.turn_zone}")
  time.sleep(0.2)

  # --- 继续上行音频（模拟用户继续说话直到说完） ---
  for i in range(8):
    session.send_audio(SILENCE)
    time.sleep(0.02)

  # --- 服务端 VAD 判停：mock 在 commit 或足够帧后发 completed ---
  session.commit_audio()
  time.sleep(0.8)

  completed = [e for e in collected if (e.get("event") or {}).get("type")
               == "conversation.item.input_audio_transcription.completed"]
  stamped = [e for e in collected if e.get("type") == "local.zone_stamp"]
  outgoing = [e for e in collected if e.get("type") == "local.outgoing"]
  outgoing_types = [(e.get("event") or {}).get("type") for e in outgoing]
  print(f"\n== completed events: {len(completed)}")
  print(f"== zone_stamp events: {json.dumps(stamped, ensure_ascii=False)}")
  print(f"== outgoing sequence: {outgoing_types}")
  print(f"== session closed: {session.closed.is_set()}")

  assert completed, "conversation.item.input_audio_transcription.completed was NOT forwarded!"
  # revert 后的行为：turn_zone 在 started 时快照，轮中切音区不会更新它，盖章用旧音区。
  assert stamped and stamped[0]["zone"] == "主驾", "reverted: stamp should keep the started-time zone"
  assert not session.closed.is_set(), "reader thread should still be alive"

  # --- 线上顺序验证：session.update 应插在音频帧之间，音频帧不中断 ---
  types_in_order = mock.received_types
  print(f"\n== upstream received sequence: {types_in_order}")
  idx_update = types_in_order.index("session.update")
  appends_before = sum(1 for t in types_in_order[:idx_update] if t == "input_audio_buffer.append")
  appends_after = sum(1 for t in types_in_order[idx_update:] if t == "input_audio_buffer.append")
  print(f"== appends before update: {appends_before}, after update: {appends_after}")
  assert appends_before > 0, "session.update should land mid-turn (after started-triggering frames)"
  assert appends_after > 0, "audio frames stopped after session.update!"

  # --- 静音/取消静音：应发送 mute/unmute 保活事件 ---
  session.send_mute()
  session.send_unmute()
  time.sleep(0.3)
  assert "input_audio_mute.commit" in mock.received_types, "mute commit not sent"
  assert "input_audio_unmute.commit" in mock.received_types, "unmute commit not sent"
  print("== mute/unmute keep-alive events OK")

  stop.set()
  session.close()
  print("\nALL ASSERTIONS PASSED")


if __name__ == "__main__":
  main()
