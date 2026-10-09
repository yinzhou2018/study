"""A2A gRPC 服务器冒烟测试：不依赖真实 LLM，使用 FakeLLMClient 注入。"""
import json
import shutil
import tempfile
import threading
import time
import unittest
from concurrent import futures
from pathlib import Path

import grpc

from a2a import a2a_pb2, a2a_pb2_grpc
from a2a_agent import TaskStore, extract_text, extract_zone
from a2a_server import A2AServiceServicer
from llm_client import MockLLMClient


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


class BlockingFakeLLM:
  """阻塞式 LLM：进入 chat 后等待 release 事件，便于构造并行请求窗口"""

  def __init__(self, reply="处理完成"):
    self.reply = reply
    self.started = threading.Event()
    self.release = threading.Event()

  def chat(self, messages, tools, temperature=0.1, effort=None):
    self.started.set()
    self.release.wait(timeout=5)
    return {
        "choices": [{
            "message": {"role": "assistant", "content": self.reply},
            "finish_reason": "stop",
        }]
    }

  def chat_stream(self, messages, tools, temperature=0.1, effort=None):
    self.started.set()
    self.release.wait(timeout=5)
    yield {"type": "done",
           "message": {"role": "assistant", "content": self.reply}}


class InputRequestFakeLLM:
  """首次调用返回 request_user_input 工具调用，之后返回固定文本回复"""

  def __init__(self, question="请问您具体想要什么操作？", reply="好的，已处理。"):
    self.question = question
    self.reply = reply
    self._calls = 0

  def chat(self, messages, tools, temperature=0.1, effort=None):
    self._calls += 1
    if self._calls > 1:
      return {
          "choices": [{
              "message": {"role": "assistant", "content": self.reply},
              "finish_reason": "stop",
          }]
      }
    return {
        "choices": [{
            "message": {
                "role": "assistant",
                "content": None,
                "tool_calls": [{
                    "id": "call_req_1",
                    "type": "function",
                    "function": {
                        "name": "request_user_input",
                        "arguments": json.dumps(
                            {"question": self.question}, ensure_ascii=False)
                    }
                }]
            },
            "finish_reason": "tool_calls"
        }]
    }

  def chat_stream(self, messages, tools, temperature=0.1, effort=None):
    # 复用 chat() 决策逻辑
    resp = self.chat(messages, tools, temperature, effort)
    msg = resp["choices"][0]["message"]
    content = msg.get("content")
    if content:
      for ch in content:
        yield {"type": "content", "text": ch}
    yield {"type": "done", "message": msg}


def _start_server(skills_dir=None, llm=None):
  """启动 in-process gRPC 服务器，返回 (server, port, store)"""
  tmp = Path(tempfile.mkdtemp()) if not skills_dir else Path(skills_dir)
  store = TaskStore(max_contexts=3, skills_dir=tmp)
  store.set_llm(llm or FakeLLMClient("测试回复"))
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


# ─────────────────────────────────────────────────────────────────────────────
# task_id / context_id 解析与校验（SendMessage）
# ─────────────────────────────────────────────────────────────────────────────

class TaskIdResolutionTest(unittest.TestCase):
  """SendMessage 的 task_id/context_id 解析规则"""

  def setUp(self):
    self.server, self.port, self.store = _start_server()
    self.stub = a2a_pb2_grpc.A2AServiceStub(_channel(self.port))

  def tearDown(self):
    self.server.stop(0)
    shutil.rmtree(self.store.skills_dir, ignore_errors=True)

  def _send(self, text, task_id="", context_id="", message_id="m"):
    msg = a2a_pb2.Message(
        message_id=message_id,
        task_id=task_id,
        context_id=context_id,
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text=text)],
    )
    return self.stub.SendMessage(a2a_pb2.SendMessageRequest(message=msg))

  @staticmethod
  def _status_text(task):
    return "".join(p.text or "" for p in task.status.message.parts)

  def _seed_completed_task(self, context_id="ctx_seed"):
    """先发一条消息制造一个已完成的任务"""
    resp = self._send("第一轮", context_id=context_id, message_id="m_seed")
    self.assertEqual(
        resp.task.status.state, a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    return resp.task

  def test_unknown_task_id_returns_failed(self):
    resp = self._send("你好", task_id="ghost_task")
    self.assertEqual(resp.task.status.state, a2a_pb2.TaskState.TASK_STATE_FAILED)
    self.assertEqual(resp.task.id, "ghost_task")
    self.assertIn("任务不存在", self._status_text(resp.task))
    # 失败响应不写入 store
    self.assertIsNone(self.store.get_task("ghost_task"))

  def test_terminal_task_id_returns_failed_and_keeps_original(self):
    task = self._seed_completed_task()
    resp = self._send("再来一轮", task_id=task.id)
    self.assertEqual(resp.task.status.state, a2a_pb2.TaskState.TASK_STATE_FAILED)
    self.assertEqual(resp.task.context_id, "ctx_seed")
    self.assertIn("任务已结束", self._status_text(resp.task))
    # 原任务状态不被覆盖
    stored = self.store.get_task(task.id)
    self.assertEqual(stored.status.state, a2a_pb2.TaskState.TASK_STATE_COMPLETED)

  def test_context_id_mismatch_returns_failed(self):
    task = self._seed_completed_task(context_id="ctx_real")
    resp = self._send("继续", task_id=task.id, context_id="ctx_wrong")
    self.assertEqual(resp.task.status.state, a2a_pb2.TaskState.TASK_STATE_FAILED)
    self.assertIn("不匹配", self._status_text(resp.task))
    # 失败响应的 context_id 取任务真实绑定的值
    self.assertEqual(resp.task.context_id, "ctx_real")

  def test_matched_context_but_terminal_returns_failed(self):
    task = self._seed_completed_task(context_id="ctx_match")
    resp = self._send("继续", task_id=task.id, context_id="ctx_match")
    self.assertEqual(resp.task.status.state, a2a_pb2.TaskState.TASK_STATE_FAILED)
    self.assertIn("任务已结束", self._status_text(resp.task))

  def test_task_id_infers_context_and_continues(self):
    # 手动放入非终态任务（SUBMITTED），模拟进行中任务
    task = a2a_pb2.Task(
        id="task_live",
        context_id="ctx_live",
        status=a2a_pb2.TaskStatus(
            state=a2a_pb2.TaskState.TASK_STATE_SUBMITTED),
    )
    self.store.put_task(task)
    agent_before = self.store.get_or_create_agent("ctx_live")

    # 仅传 task_id，服务端推断 context_id 并复用 agent
    resp = self._send("补充输入", task_id="task_live")
    self.assertEqual(resp.task.id, "task_live")
    self.assertEqual(resp.task.context_id, "ctx_live")
    self.assertEqual(resp.task.status.state,
                     a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    # history 累积：user + agent
    self.assertEqual(len(resp.task.history), 2)
    self.assertEqual(resp.task.history[0].role, a2a_pb2.Role.ROLE_USER)
    self.assertEqual(resp.task.history[-1].role, a2a_pb2.Role.ROLE_AGENT)
    # 推断出的 context 复用同一 agent
    self.assertIs(self.store._agents["ctx_live"], agent_before)
    user_msgs = [m for m in agent_before.messages if m["role"] == "user"]
    self.assertEqual(len(user_msgs), 1)

  def test_task_and_context_match_continues_with_history(self):
    # 两者都传且匹配非终态任务（INPUT_REQUIRED 属可继续的中断态）
    task = a2a_pb2.Task(
        id="task_pair",
        context_id="ctx_pair",
        status=a2a_pb2.TaskStatus(
            state=a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED),
    )
    self.store.put_task(task)

    resp = self._send("继续补充", task_id="task_pair", context_id="ctx_pair")
    self.assertEqual(resp.task.status.state,
                     a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    self.assertEqual(len(resp.task.history), 2)


# ─────────────────────────────────────────────────────────────────────────────
# INPUT_REQUIRED 生命周期（request_user_input → task_id 续接闭环）
# ─────────────────────────────────────────────────────────────────────────────

class InputRequiredTest(unittest.TestCase):
  """模型请求补充输入时任务转 INPUT_REQUIRED，客户端用 task_id 续接后完成"""

  def setUp(self):
    self.llm = InputRequestFakeLLM()
    self.server, self.port, self.store = _start_server(llm=self.llm)
    self.stub = a2a_pb2_grpc.A2AServiceStub(_channel(self.port))

  def tearDown(self):
    self.server.stop(0)
    shutil.rmtree(self.store.skills_dir, ignore_errors=True)

  @staticmethod
  def _message(text, task_id="", context_id="", message_id="m"):
    return a2a_pb2.Message(
        message_id=message_id,
        task_id=task_id,
        context_id=context_id,
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text=text)],
    )

  def test_send_message_needs_input_then_continue(self):
    # 第一轮：歧义指令 → 模型调用 request_user_input → INPUT_REQUIRED
    resp1 = self.stub.SendMessage(a2a_pb2.SendMessageRequest(
        message=self._message("这个指令有点歧义", message_id="m1")))
    task1 = resp1.task
    self.assertEqual(task1.status.state,
                     a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED)
    question = "".join(p.text or "" for p in task1.status.message.parts)
    self.assertIn("具体想要什么操作", question)
    # history：user + agent（澄清问题）
    self.assertEqual(len(task1.history), 2)
    self.assertEqual(task1.history[-1].role, a2a_pb2.Role.ROLE_AGENT)

    # 第二轮：仅用 task_id 续接（推断 context_id），补充输入后完成
    resp2 = self.stub.SendMessage(a2a_pb2.SendMessageRequest(
        message=self._message("打开空调制冷", task_id=task1.id,
                              message_id="m2")))
    task2 = resp2.task
    self.assertEqual(task2.id, task1.id)
    self.assertEqual(task2.context_id, task1.context_id)
    self.assertEqual(task2.status.state, a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    # 同一任务 history 累积：2 + 2 = 4
    self.assertEqual(len(task2.history), 4)

  def test_streaming_needs_input_then_continue(self):
    # 第一轮流式：初始 Task → 终态 INPUT_REQUIRED
    events = list(self.stub.SendStreamingMessage(a2a_pb2.SendMessageRequest(
        message=self._message("这个指令有点歧义", message_id="ms1"))))
    self.assertEqual(events[0].WhichOneof("payload"), "task")
    last = events[-1]
    self.assertEqual(last.status_update.status.state,
                     a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED)
    task_id = last.status_update.task_id

    # 第二轮流式：task_id 续接 → [WORKING, COMPLETED] status_update，不发 Task 事件
    events2 = list(self.stub.SendStreamingMessage(a2a_pb2.SendMessageRequest(
        message=self._message("打开空调制冷", task_id=task_id,
                              message_id="ms2"))))
    self.assertEqual(len(events2), 2)
    for e in events2:
      self.assertEqual(e.WhichOneof("payload"), "status_update")
    self.assertEqual(events2[0].status_update.status.state,
                     a2a_pb2.TaskState.TASK_STATE_WORKING)
    self.assertEqual(events2[-1].status_update.status.state,
                     a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    reply = "".join(p.text or "" for p in
                    events2[-1].status_update.status.message.parts)
    self.assertIn("好的，已处理", reply)

  def test_mock_llm_full_loop(self):
    """MockLLMClient 完整闭环：歧义 → INPUT_REQUIRED → 补充 → COMPLETED"""
    server, port, store = _start_server(llm=MockLLMClient())
    try:
      stub = a2a_pb2_grpc.A2AServiceStub(_channel(port))
      resp1 = stub.SendMessage(a2a_pb2.SendMessageRequest(
          message=self._message("这个指令有点歧义", message_id="mm1")))
      self.assertEqual(resp1.task.status.state,
                       a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED)

      resp2 = stub.SendMessage(a2a_pb2.SendMessageRequest(
          message=self._message("打开空调制冷", task_id=resp1.task.id,
                                message_id="mm2")))
      self.assertEqual(resp2.task.status.state,
                       a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    finally:
      server.stop(0)
      shutil.rmtree(store.skills_dir, ignore_errors=True)


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
    self.assertEqual(len(events), 2)
    # 首事件为 Task 对象（WORKING 状态，history 含用户消息）
    self.assertEqual(events[0].WhichOneof("payload"), "task")
    self.assertEqual(events[0].task.status.state,
                     a2a_pb2.TaskState.TASK_STATE_WORKING)
    self.assertEqual(events[0].task.history[-1].role, a2a_pb2.Role.ROLE_USER)
    # 后续事件全部为 TaskStatusUpdateEvent
    for e in events[1:]:
      self.assertEqual(e.WhichOneof("payload"), "status_update")
    # 末事件为 COMPLETED 终态，status.message 携带回复
    last = events[-1]
    self.assertEqual(last.status_update.status.state,
                     a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    self.assertEqual(last.status_update.status.message.role,
                     a2a_pb2.Role.ROLE_AGENT)
    self.assertIn("测试回复", self._final_text(events))

  def test_streaming_unknown_task_id_yields_failed_events(self):
    events = self._stream("你好", task_id="ghost_stream")
    # 解析失败仅发 TaskStatusUpdateEvent，不发 Task 事件
    self.assertEqual(len(events), 1)
    for e in events:
      self.assertEqual(e.WhichOneof("payload"), "status_update")
    last = events[-1]
    self.assertEqual(last.status_update.status.state,
                     a2a_pb2.TaskState.TASK_STATE_FAILED)
    self.assertIn("任务不存在", self._final_text(events))
    # 失败不写入 store
    self.assertIsNone(self.store.get_task("ghost_stream"))

  def test_streaming_terminal_task_id_yields_failed_events(self):
    # 先同步发送一条消息，制造一个已完成任务
    msg = a2a_pb2.Message(
        message_id="ms_seed",
        context_id="ctx_stream",
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text="第一轮")],
    )
    resp = self.stub.SendMessage(a2a_pb2.SendMessageRequest(message=msg))
    task_id = resp.task.id

    events = self._stream("第二轮", task_id=task_id)
    # 续接已终态任务仅发 FAILED TaskStatusUpdateEvent，不发 Task 事件
    self.assertEqual(len(events), 1)
    last = events[-1]
    self.assertEqual(last.WhichOneof("payload"), "status_update")
    self.assertEqual(last.status_update.status.state,
                     a2a_pb2.TaskState.TASK_STATE_FAILED)
    self.assertIn("任务已结束", self._final_text(events))
    # 原任务仍为 COMPLETED
    self.assertEqual(
        self.store.get_task(task_id).status.state,
        a2a_pb2.TaskState.TASK_STATE_COMPLETED)

  def test_streaming_continues_non_terminal_task(self):
    # 非终态任务（INPUT_REQUIRED）接收流式消息应继续执行
    task = a2a_pb2.Task(
        id="task_live_s",
        context_id="ctx_live_s",
        status=a2a_pb2.TaskStatus(
            state=a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED),
    )
    self.store.put_task(task)

    events = self._stream("流式补充", task_id="task_live_s")
    # 续接已有任务：[WORKING, COMPLETED] status_update，不发 Task 事件
    self.assertEqual(len(events), 2)
    for e in events:
      self.assertEqual(e.WhichOneof("payload"), "status_update")
    self.assertEqual(events[0].status_update.status.state,
                     a2a_pb2.TaskState.TASK_STATE_WORKING)
    last = events[-1]
    self.assertEqual(last.status_update.status.state,
                     a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    self.assertEqual(last.status_update.task_id, "task_live_s")
    self.assertEqual(last.status_update.context_id, "ctx_live_s")
    # 任务历史累积 user + agent
    stored = self.store.get_task("task_live_s")
    self.assertEqual(len(stored.history), 2)

  # ---------- 测试辅助 ----------

  def _stream(self, text, task_id="", context_id="", message_id="ms"):
    """发送流式消息并收集全部事件"""
    msg = a2a_pb2.Message(
        message_id=message_id,
        task_id=task_id,
        context_id=context_id,
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text=text)],
    )
    return list(self.stub.SendStreamingMessage(
        a2a_pb2.SendMessageRequest(message=msg)))

  @staticmethod
  def _final_text(events):
    """提取末事件 TaskStatusUpdateEvent 携带的 message 文本"""
    status = events[-1].status_update.status
    return "".join(p.text or "" for p in status.message.parts)


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


# ─────────────────────────────────────────────────────────────────────────────
# 并发保护：同一 context busy 守卫
# ─────────────────────────────────────────────────────────────────────────────

class ConcurrencyTest(unittest.TestCase):
  """同一 context 并发请求仅处理一个，其余返回 FAILED"""

  def setUp(self):
    self.llm = BlockingFakeLLM()
    self.server, self.port, self.store = _start_server(llm=self.llm)
    self.stub = a2a_pb2_grpc.A2AServiceStub(_channel(self.port))

  def tearDown(self):
    self.llm.release.set()  # 防止线程卡死
    self.server.stop(0)
    shutil.rmtree(self.store.skills_dir, ignore_errors=True)

  @staticmethod
  def _final_state(events):
    return events[-1].status_update.status.state

  def _message(self, text, context_id, message_id):
    return a2a_pb2.Message(
        message_id=message_id,
        context_id=context_id,
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text=text)],
    )

  def test_parallel_same_context_one_rejected(self):
    results = {}

    def call(tag):
      resp = self.stub.SendMessage(a2a_pb2.SendMessageRequest(
          message=self._message("你好", "ctx_par", "m_" + tag)))
      results[tag] = resp.task.status.state

    t1 = threading.Thread(target=call, args=("a",))
    t1.start()
    # 等待 t1 进入 chat（已持有 busy 锁），再发起并行请求
    self.llm.started.wait(timeout=5)
    t2 = threading.Thread(target=call, args=("b",))
    t2.start()
    t2.join(timeout=5)  # t2 应被立即拒绝
    self.llm.release.set()  # 释放 t1
    t1.join(timeout=5)

    self.assertEqual(results["a"], a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    self.assertEqual(results["b"], a2a_pb2.TaskState.TASK_STATE_FAILED)

  def test_parallel_same_context_rejected_message(self):
    results = []

    def call():
      resp = self.stub.SendMessage(a2a_pb2.SendMessageRequest(
          message=self._message("你好", "ctx_rj", "m_rj")))
      results.append(resp)

    t1 = threading.Thread(target=call)
    t1.start()
    self.llm.started.wait(timeout=5)
    t2 = threading.Thread(target=call)
    t2.start()
    t2.join(timeout=5)
    self.llm.release.set()
    t1.join(timeout=5)

    # 被拒方返回 FAILED 且 message 为拒绝说明
    rejected = [r for r in results if r.task.status.state
                == a2a_pb2.TaskState.TASK_STATE_FAILED]
    self.assertEqual(len(rejected), 1)
    text = "".join(p.text or "" for p in rejected[0].task.status.message.parts)
    self.assertIn("任务处理中", text)

  def test_streaming_parallel_same_context_one_rejected(self):
    buckets = {}

    def call(tag):
      buckets[tag] = list(self.stub.SendStreamingMessage(
          a2a_pb2.SendMessageRequest(
              message=self._message("你好", "ctx_sp", "ms_" + tag))))

    t1 = threading.Thread(target=call, args=("a",))
    t1.start()
    self.llm.started.wait(timeout=5)
    t2 = threading.Thread(target=call, args=("b",))
    t2.start()
    t2.join(timeout=5)
    self.llm.release.set()
    t1.join(timeout=5)

    # 赢家：新任务 [task, COMPLETED]；被拒方：仅 [FAILED status_update]
    self.assertEqual(self._final_state(buckets["a"]),
                     a2a_pb2.TaskState.TASK_STATE_COMPLETED)
    self.assertEqual(self._final_state(buckets["b"]),
                     a2a_pb2.TaskState.TASK_STATE_FAILED)
    self.assertEqual(len(buckets["b"]), 1)
    self.assertEqual(buckets["b"][0].WhichOneof("payload"), "status_update")

  def test_parallel_different_contexts_both_succeed(self):
    self.llm.release.set()  # chat 直接返回，无需同步
    results = []

    def call(ctx):
      resp = self.stub.SendMessage(a2a_pb2.SendMessageRequest(
          message=self._message("你好", ctx, "m_" + ctx)))
      results.append(resp.task.status.state)

    t1 = threading.Thread(target=call, args=("ctx_a",))
    t2 = threading.Thread(target=call, args=("ctx_b",))
    t1.start()
    t2.start()
    t1.join(timeout=5)
    t2.join(timeout=5)

    # 不同 context 不互斥，两个均成功
    self.assertEqual(sorted(results), [
        a2a_pb2.TaskState.TASK_STATE_COMPLETED,
        a2a_pb2.TaskState.TASK_STATE_COMPLETED,
    ])


if __name__ == "__main__":
  unittest.main()
