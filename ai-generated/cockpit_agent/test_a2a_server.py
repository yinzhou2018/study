"""A2A gRPC 服务器冒烟测试：不依赖真实 LLM，使用 FakeLLMClient 注入。"""
import shutil
import tempfile
import unittest
from concurrent import futures
from pathlib import Path

import grpc

from a2a import a2a_pb2, a2a_pb2_grpc
from a2a_agent import TaskStore, extract_text, extract_zone
from a2a_server import A2AServiceServicer


# ──────────────────────────────────────────────────────────────────────────────
# 辅助工具
# ──────────────────────────────────────────────────────────────────────────────

class FakeLLMClient:
  """返回固定回复的 LLM，用于测试（不发起真实网络请求）"""

  def __init__(self, reply: str = "好的，已处理。"):
    self.reply = reply

  def chat(self, messages, tools, temperature=0.1, effort=None):
    return {
        "choices": [{
            "message": {"role": "assistant", "content": self.reply},
            "finish_reason": "stop",
        }]
    }

  def chat_stream(self, messages, tools, temperature=0.1, effort=None):
    # 模拟流式：逐 token 返回
    for ch in self.reply:
      yield {"type": "content", "text": ch}
    yield {"type": "done", "message": {"role": "assistant", "content": self.reply}}


def _start_server(skills_dir=None):
  """启动 in-process gRPC 服务器，返回 (server, port, store)"""
  tmp = Path(tempfile.mkdtemp()) if not skills_dir else Path(skills_dir)
  store = TaskStore(max_contexts=3, skills_dir=tmp)
  store.set_llm(FakeLLMClient("测试回复"))
  servicer = A2AServiceServicer(store, address=f"127.0.0.1:50051")
  server = grpc.server(futures.ThreadPoolExecutor(max_workers=4))
  a2a_pb2_grpc.add_A2AServiceServicer_to_server(servicer, server)
  port = server.add_insecure_port("127.0.0.1:0")
  server.start()
  return server, port, store


def _channel(port):
  return grpc.insecure_channel(f"127.0.0.1:{port}")


# ──────────────────────────────────────────────────────────────────────────────
# AgentCard
# ──────────────────────────────────────────────────────────────────────────────

class AgentCardTest(unittest.TestCase):

  def setUp(self):
    self.server, self.port, self.store = _start_server()
    self.stub = a2a_pb2_grpc.A2AServiceStub(_channel(self.port))

  def tearDown(self):
    self.server.stop(0)
    shutil.rmtree(self.store.skills_dir, ignore_errors=True)

  def test_agent_card_name(self):
    card = self.stub.GetExtendedAgentCard(
        a2a_pb2.GetExtendedAgentCardRequest(tenant="default")
    )
    self.assertEqual(card.name, "智能座舱助手")
    self.assertEqual(card.version, "1.0.0")
    self.assertTrue(card.capabilities.streaming)
    self.assertGreater(len(card.skills), 0)
    self.assertEqual(card.default_input_modes[0], "text/plain")

  def test_agent_card_skills_from_toolsets(self):
    card = self.stub.GetExtendedAgentCard(
        a2a_pb2.GetExtendedAgentCardRequest(tenant="default")
    )
    # 工具集派生的技能名应在 skills 列表中
    skill_ids = [s.id for s in card.skills]
    self.assertIn("toolset_body_control", skill_ids)


# ──────────────────────────────────────────────────────────────────────────────
# SendMessage
# ──────────────────────────────────────────────────────────────────────────────

class SendMessageTest(unittest.TestCase):

  def setUp(self):
    self.server, self.port, self.store = _start_server()
    self.stub = a2a_pb2_grpc.A2AServiceStub(_channel(self.port))

  def tearDown(self):
    self.server.stop(0)
    shutil.rmtree(self.store.skills_dir, ignore_errors=True)

  def test_send_message_returns_completed_task(self):
    msg = a2a_pb2.Message(
        message_id="m1",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="你好")],
    )
    req = a2a_pb2.SendMessageRequest(message=msg)
    resp = self.stub.SendMessage(req)
    self.assertEqual(resp.task.status.state, a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    self.assertGreater(len(resp.task.history), 0)
    # 最后一条 history 是 agent 回复
    self.assertEqual(resp.task.history[-1].role, a2a_pb2.Role.ROLE_AGENT)

  def test_send_message_empty_text_returns_invalid_argument(self):
    msg = a2a_pb2.Message(
        message_id="m_empty",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[],  # 空 parts
    )
    req = a2a_pb2.SendMessageRequest(message=msg)
    with self.assertRaises(grpc.RpcError) as ctx:
      self.stub.SendMessage(req)
    self.assertEqual(ctx.exception.code(), grpc.StatusCode.INVALID_ARGUMENT)

  def test_send_message_reuse_context(self):
    ctx_id = "ctx_test_123"
    msg = a2a_pb2.Message(
        message_id="m1",
        context_id=ctx_id,
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="第一轮")],
    )
    req = a2a_pb2.SendMessageRequest(message=msg)
    resp1 = self.stub.SendMessage(req)
    self.assertEqual(resp1.task.context_id, ctx_id)
    # 每个 Task 的 history 只含自己的消息
    self.assertEqual(len(resp1.task.history), 2)

    # 第二轮同 context_id
    msg2 = a2a_pb2.Message(
        message_id="m2",
        context_id=ctx_id,
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="第二轮")],
    )
    resp2 = self.stub.SendMessage(a2a_pb2.SendMessageRequest(message=msg2))
    self.assertEqual(resp2.task.context_id, ctx_id)
    self.assertEqual(len(resp2.task.history), 2)
    # 同 context 共享同一个 CockpitAgent，内部对话历史已累积 2 轮
    agent = self.store._agents[ctx_id]
    user_msgs = [m for m in agent.messages if m["role"] == "user"]
    self.assertEqual(len(user_msgs), 2)


# ──────────────────────────────────────────────────────────────────────────────
# SendStreamingMessage
# ──────────────────────────────────────────────────────────────────────────────

class StreamingTest(unittest.TestCase):

  def setUp(self):
    self.server, self.port, self.store = _start_server()
    self.stub = a2a_pb2_grpc.A2AServiceStub(_channel(self.port))

  def tearDown(self):
    self.server.stop(0)
    shutil.rmtree(self.store.skills_dir, ignore_errors=True)

  def test_streaming_message_returns_events(self):
    msg = a2a_pb2.Message(
        message_id="ms1",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="流式测试")],
    )
    req = a2a_pb2.SendMessageRequest(message=msg)
    events = list(self.stub.SendStreamingMessage(req))
    self.assertGreaterEqual(len(events), 3)
    # 第一个事件是 WORKING 状态
    self.assertEqual(events[0].status_update.status.state,
                     a2a_pb2.TaskState.TASK_STATE_WORKING)
    # 最后一个事件是 COMPLETED 状态
    last = events[-1]
    self.assertEqual(last.status_update.status.state,
                     a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    # 中间应有 message 事件
    msg_events = [e for e in events if e.WhichOneof("payload") == "message"]
    self.assertEqual(len(msg_events), 1)
    self.assertEqual(msg_events[0].message.role, a2a_pb2.Role.ROLE_AGENT)
    self.assertIn("测试回复", "".join(p.text for p in msg_events[0].message.parts))


# ──────────────────────────────────────────────────────────────────────────────
# GetTask
# ──────────────────────────────────────────────────────────────────────────────

class GetTaskTest(unittest.TestCase):

  def setUp(self):
    self.server, self.port, self.store = _start_server()
    self.stub = a2a_pb2_grpc.A2AServiceStub(_channel(self.port))

  def tearDown(self):
    self.server.stop(0)
    shutil.rmtree(self.store.skills_dir, ignore_errors=True)

  def test_get_task_found(self):
    # 先发送一个消息创建 task
    msg = a2a_pb2.Message(
        message_id="mt1",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="查询任务")],
    )
    resp = self.stub.SendMessage(a2a_pb2.SendMessageRequest(message=msg))
    task_id = resp.task.id
    task = self.stub.GetTask(a2a_pb2.GetTaskRequest(id=task_id))
    self.assertEqual(task.id, task_id)
    self.assertEqual(task.status.state, a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    self.assertGreater(len(task.history), 0)

  def test_get_task_not_found(self):
    with self.assertRaises(grpc.RpcError) as ctx:
      self.stub.GetTask(a2a_pb2.GetTaskRequest(id="nonexistent"))
    self.assertEqual(ctx.exception.code(), grpc.StatusCode.NOT_FOUND)

  def test_get_task_history_length_limit(self):
    msg = a2a_pb2.Message(
        message_id="mt2",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="带历史长度")],
    )
    resp = self.stub.SendMessage(a2a_pb2.SendMessageRequest(message=msg))
    task_id = resp.task.id
    # 请求只返回最近 1 条 history
    task = self.stub.GetTask(
        a2a_pb2.GetTaskRequest(id=task_id, history_length=1)
    )
    self.assertEqual(len(task.history), 1)


# ──────────────────────────────────────────────────────────────────────────────
# CancelTask
# ──────────────────────────────────────────────────────────────────────────────

class CancelTaskTest(unittest.TestCase):

  def setUp(self):
    self.server, self.port, self.store = _start_server()
    self.stub = a2a_pb2_grpc.A2AServiceStub(_channel(self.port))

  def tearDown(self):
    self.server.stop(0)
    shutil.rmtree(self.store.skills_dir, ignore_errors=True)

  def test_cancel_completed_task_returns_same_task(self):
    msg = a2a_pb2.Message(
        message_id="mc1",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="取消测试")],
    )
    resp = self.stub.SendMessage(a2a_pb2.SendMessageRequest(message=msg))
    task_id = resp.task.id
    # 已完成的任务取消，应返回同一任务（不改变状态）
    task = self.stub.CancelTask(a2a_pb2.CancelTaskRequest(id=task_id))
    self.assertEqual(task.status.state, a2a_pb2.TaskState.TASK_STATE_COMPLETED)

  def test_cancel_nonexistent_task_not_found(self):
    with self.assertRaises(grpc.RpcError) as ctx:
      self.stub.CancelTask(a2a_pb2.CancelTaskRequest(id="ghost"))
    self.assertEqual(ctx.exception.code(), grpc.StatusCode.NOT_FOUND)


# ──────────────────────────────────────────────────────────────────────────────
# TaskStore LRU 淘汰
# ──────────────────────────────────────────────────────────────────────────────

class TaskStoreLruTest(unittest.TestCase):

  def test_lru_eviction(self):
    tmp = Path(tempfile.mkdtemp())
    store = TaskStore(max_contexts=2, skills_dir=tmp)
    store.set_llm(FakeLLMClient())

    a1 = store.get_or_create_agent("ctx1")
    a2 = store.get_or_create_agent("ctx2")
    # ctx1 更久未用，创建 ctx3 应淘汰 ctx1
    store.get_or_create_agent("ctx3")

    self.assertNotIn("ctx1", store._agents)
    self.assertIn("ctx2", store._agents)
    self.assertIn("ctx3", store._agents)

    # 更新 ctx2 的使用顺序，再次创建应淘汰 ctx3
    store.touch_agent("ctx2")
    store.get_or_create_agent("ctx4")
    self.assertNotIn("ctx3", store._agents)
    self.assertIn("ctx2", store._agents)
    self.assertIn("ctx4", store._agents)
    shutil.rmtree(tmp)


# ──────────────────────────────────────────────────────────────────────────────
# 工具函数
# ──────────────────────────────────────────────────────────────────────────────

class ExtractHelpersTest(unittest.TestCase):

  def test_extract_text_single(self):
    msg = a2a_pb2.Message(
        message_id="e1",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="hello")],
    )
    self.assertEqual(extract_text(msg), "hello")

  def test_extract_text_multiple(self):
    msg = a2a_pb2.Message(
        message_id="e2",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="a"), a2a_pb2.Part(text="b")],
    )
    self.assertEqual(extract_text(msg), "ab")

  def test_extract_zone_from_metadata(self):
    msg = a2a_pb2.Message(
        message_id="z1",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="x")],
    )
    msg.metadata["zone"] = "rear_right"
    self.assertEqual(extract_zone(msg), "rear_right")

  def test_extract_zone_default(self):
    msg = a2a_pb2.Message(
        message_id="z2",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="x")],
    )
    self.assertEqual(extract_zone(msg), "front_left")


if __name__ == "__main__":
  unittest.main()
