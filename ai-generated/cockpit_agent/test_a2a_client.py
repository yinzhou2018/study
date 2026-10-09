"""a2a_client 交互客户端测试：不依赖真实 LLM，复用 in-process gRPC 服务器。"""
import contextlib
import io
import shutil
import unittest

from a2a import a2a_pb2, a2a_pb2_grpc
from a2a_client import A2AClient, A2AClientSession
from test_a2a_server import (
    FakeLLMClient,
    InputRequestFakeLLM,
    _channel,
    _start_server,
)


class A2AClientSessionTest(unittest.TestCase):
  """会话状态：INPUT_REQUIRED 续接判定与重置"""

  def test_should_continue_after_input_required(self):
    s = A2AClientSession()
    self.assertFalse(s.should_continue_task())
    s.last_terminal = a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED
    s.last_task_id = "t1"
    self.assertTrue(s.should_continue_task())
    # 其他终态不续接
    s.last_terminal = a2a_pb2.TaskState.TASK_STATE_COMPLETED
    self.assertFalse(s.should_continue_task())

  def test_reset(self):
    s = A2AClientSession()
    old_ctx = s.context_id
    s.last_terminal = a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED
    s.last_task_id = "t1"
    s.reset()
    self.assertNotEqual(s.context_id, old_ctx)
    self.assertFalse(s.should_continue_task())
    self.assertEqual(s.last_task_id, "")


class A2AClientTest(unittest.TestCase):
  """新任务流：发送 → 终态 COMPLETED + 回复打印"""

  def setUp(self):
    self.server, self.port, self.store = _start_server()
    self.client = A2AClient(a2a_pb2_grpc.A2AServiceStub(_channel(self.port)))

  def tearDown(self):
    self.server.stop(0)
    shutil.rmtree(self.store.skills_dir, ignore_errors=True)

  def _send(self, text):
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
      self.client._send(text)
    return buf.getvalue()

  def test_new_task_completed(self):
    out = self._send("你好")
    self.assertEqual(self.client.session.last_terminal,
                     a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    self.assertIn("测试回复", out)
    self.assertIn("助手", out)
    self.assertTrue(self.client.session.last_task_id)

  def test_two_rounds_new_tasks_share_context(self):
    self._send("第一轮")
    self._send("第二轮")
    # 两个新任务共用同一 context（同一 agent 累积两轮用户消息）
    agent = self.store._agents[self.client.session.context_id]
    user_msgs = [m for m in agent.messages if m["role"] == "user"]
    self.assertEqual(len(user_msgs), 2)

  def test_zone_command(self):
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
      self.client._handle_command("/zone rear_right")
    self.assertEqual(self.client.session.zone, "rear_right")
    with contextlib.redirect_stdout(io.StringIO()):
      self.client._handle_command("/zone bogus")
    self.assertEqual(self.client.session.zone, "rear_right")

  def test_clear_command_resets_session(self):
    self._send("你好")
    old_ctx = self.client.session.context_id
    with contextlib.redirect_stdout(io.StringIO()):
      self.client._handle_command("/clear")
    self.assertNotEqual(self.client.session.context_id, old_ctx)
    self.assertFalse(self.client.session.should_continue_task())


class A2AClientContinueTest(unittest.TestCase):
  """INPUT_REQUIRED 自动续接闭环"""

  def setUp(self):
    self.llm = InputRequestFakeLLM()
    self.server, self.port, self.store = _start_server(llm=self.llm)
    self.client = A2AClient(a2a_pb2_grpc.A2AServiceStub(_channel(self.port)))

  def tearDown(self):
    self.server.stop(0)
    shutil.rmtree(self.store.skills_dir, ignore_errors=True)

  def _send(self, text):
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
      self.client._send(text)
    return buf.getvalue()

  def test_round1_input_required_round2_auto_continue(self):
    out1 = self._send("这个指令有点歧义")
    self.assertEqual(self.client.session.last_terminal,
                     a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED)
    self.assertIn("具体想要什么操作", out1)
    self.assertTrue(self.client.session.should_continue_task())

    # 第二轮：自动带 task_id 续接（无需用户干预）
    out2 = self._send("打开空调制冷")
    self.assertEqual(self.client.session.last_terminal,
                     a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    self.assertIn("好的，已处理", out2)

    # 同一任务被续接：history 累积两轮（user+agent）×2 = 4
    task = self.store.get_task(self.client.session.last_task_id)
    self.assertEqual(task.status.state, a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    self.assertEqual(len(task.history), 4)
    # 第三轮：已 COMPLETED，不再续接，回到新任务
    self.assertFalse(self.client.session.should_continue_task())


if __name__ == "__main__":
  unittest.main()
