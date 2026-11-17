#!/usr/bin/env python3
"""音区盖章（机制 B）「写后校验轮询」闭环测试。

背景：demo 在 transcription.completed 时立即发 conversation.item.update 盖章，
但上游可能在稍后用 ASR 原文覆盖同一 item（ack 照常返回，内容却被还原）。
服务端的对策：update 后每 100ms 发 retrieve 校验，被覆盖则重写，
直到确认盖章留存（acked）或总时长超过 10s（failed）。

场景 A（无覆盖）：update 一次即生效 → acked，无 rewritten。
场景 B（模拟上游覆盖）：mock 把首个 update 用原文覆盖 →
  服务端轮询发现 rewritten → 重写 update → 再次确认 acked。
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
from mock_upstream import MockUpstream  # noqa: E402

SILENCE = base64.b64encode(b"\x00" * 320).decode("ascii")


def drain_events(session, collected, stop):
  while not stop.is_set():
    try:
      ev = session.events.get(timeout=0.2)
    except Exception:
      continue
    collected.append(ev)


def run_turn(session, collected, mark):
  """说完一句话（触发 completed + 盖章轮询），返回终态 zone_stamp 事件（acked/failed）。"""
  for _ in range(7):
    session.send_audio(SILENCE)
    time.sleep(0.02)
  session.commit_audio()
  deadline = time.time() + 14  # 轮询总时长上限 10s + 充裕余量
  while time.time() < deadline:
    for ev in collected[mark:]:
      if ev.get("type") == "local.zone_stamp" and ev.get("status") in ("acked", "failed"):
        return ev
    time.sleep(0.05)
  return None


def check(mode, port):
  print(f"\n===== 场景 {mode} =====")
  server.MinimalWebSocketClient._open_tls_socket = (
      lambda self, verify, _p=port: socket.create_connection(("127.0.0.1", _p), timeout=20)
  )
  mock = MockUpstream(port=port)
  mock.overwrite_next_update = (mode == "B覆盖")
  threading.Thread(target=mock.serve, daemon=True).start()
  time.sleep(0.3)

  config = {
      "api_key": "test-key",
      "tool_root": os.path.expanduser("~"),
      "zone": "主驾",
      "enable_tools": False,
      "input_format": "pcm",
  }
  session = server.STATE.create(config)
  collected = []
  stop = threading.Event()
  threading.Thread(target=drain_events, args=(session, collected, stop), daemon=True).start()
  assert session.ready.wait(timeout=5), "session not ready"

  mark = len(collected)
  final = run_turn(session, collected, mark)
  stamps = [e for e in collected[mark:] if e.get("type") == "local.zone_stamp"]
  print("zone_stamp events:", json.dumps(stamps, ensure_ascii=False))
  assert final, "time limit: no terminal zone_stamp (acked/failed) event"
  assert final["status"] == "acked", f"expected acked, got: {json.dumps(final, ensure_ascii=False)}"

  updates = [e for e in collected[mark:] if e.get("type") == "local.outgoing"
             and (e.get("event") or {}).get("type") == "conversation.item.update"]
  retrieves = [e for e in collected[mark:] if e.get("type") == "local.outgoing"
               and (e.get("event") or {}).get("type") == "conversation.item.retrieve"]
  print(f"item.update count: {len(updates)}, retrieve count: {len(retrieves)}")

  stamped = [e for e in stamps if e.get("stamped")]
  assert stamped and stamped[0]["zone"] == "主驾", "stamp event with started-time zone expected"
  acked = [e for e in stamps if e.get("status") == "acked"]
  assert acked and acked[0]["text"].startswith("【主驾】"), "acked stamp text must carry zone prefix"

  if mode == "B覆盖":
    rewritten = [e for e in stamps if e.get("status") == "rewritten"]
    assert rewritten, "overwrite scenario must produce rewritten event"
    assert rewritten[0].get("expected", "").startswith("【主驾】"), "rewritten.expected is stamped text"
    assert rewritten[0].get("actual") == "现在是谁在跟你说话", "rewritten.actual is upstream original text"
    assert len(updates) >= 2, "overwrite scenario must resend item.update"
  else:
    assert not [e for e in stamps if e.get("status") == "rewritten"], "no-overwrite scenario must not rewrite"
    assert len(updates) == 1, "no-overwrite scenario needs exactly one item.update"
  assert len(retrieves) >= 1, "polling must issue at least one retrieve"

  stop.set()
  session.close()
  time.sleep(0.2)
  print(f"== 场景 {mode} OK")


if __name__ == "__main__":
  check("A无覆盖", 8902)
  check("B覆盖", 8903)
  print("\nALL ASSERTIONS PASSED")
