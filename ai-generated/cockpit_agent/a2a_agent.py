"""
A2A 协议适配层：CockpitAgent ↔ A2A Task 生命周期映射。

- TaskStore：管理多对话上下文（context_id → CockpitAgent），LRU 淘汰，上限 50。
- A2AAgent：负责 A2A Message/Task 类型与 CockpitAgent 调用之间的相互转换。
"""
import threading
import uuid
from collections import OrderedDict

from a2a import a2a_pb2
from cockpit_agent import CockpitAgent
from config import DEFAULT_ZONE


# 最大同时维护的对话上下文数
MAX_CONTEXTS = 50


def extract_text(message: a2a_pb2.Message) -> str: # type: ignore
  """从 A2A Message 中提取所有 text part，拼接为单一字符串"""
  return "".join(p.text or "" for p in message.parts)


def extract_zone(message: a2a_pb2.Message) -> str: # type: ignore
  """从 Message.metadata.zone 提取音区，默认 front_left"""
  if message.HasField("metadata") and "zone" in message.metadata:
    zone = message.metadata["zone"]
    if isinstance(zone, str) and zone:
      return zone
  return DEFAULT_ZONE


def build_agent_message(
    text: str, task_id: str = "", context_id: str = ""
) -> a2a_pb2.Message: # type: ignore
  """构建 A2A Agent 回复消息"""
  return a2a_pb2.Message( # type: ignore
      message_id=str(uuid.uuid4()),
      task_id=task_id,
      context_id=context_id,
      role=a2a_pb2.Role.ROLE_AGENT, # type: ignore
      parts=[a2a_pb2.Part(text=text)], # type: ignore
  )


class TaskStore:
  """A2A Task 存储 + 多对话上下文管理（线程安全）"""

  def __init__(self, max_contexts: int = MAX_CONTEXTS, skills_dir=None):
    self.max_contexts = max_contexts
    self.skills_dir = skills_dir
    self._lock = threading.Lock()
    self._tasks: dict[str, a2a_pb2.Task] = {} # type: ignore
    self._agents: OrderedDict[str, CockpitAgent] = OrderedDict()
    self._llm = None
    self._busy: set[str] = set()  # 正在处理的 context_id，并发保护

  def set_llm(self, llm):
    self._llm = llm

  def get_or_create_agent(self, context_id: str) -> CockpitAgent:
    with self._lock:
      if context_id in self._agents:
        self._agents.move_to_end(context_id)
        return self._agents[context_id]
      # LRU 淘汰
      while len(self._agents) >= self.max_contexts:
        self._agents.popitem(last=False)
      agent = CockpitAgent(llm_client=self._llm, skills_dir=self.skills_dir)
      self._agents[context_id] = agent
      return agent

  def put_task(self, task: a2a_pb2.Task): # type: ignore
    with self._lock:
      self._tasks[task.id] = task

  def get_task(self, task_id: str) -> a2a_pb2.Task | None: # type: ignore
    with self._lock:
      return self._tasks.get(task_id)

  def touch_agent(self, context_id: str):
    with self._lock:
      if context_id in self._agents:
        self._agents.move_to_end(context_id)

  def cancel_running_task(self, context_id: str):
    """通知正在运行的 agent 打断（best-effort）"""
    with self._lock:
      if context_id in self._agents:
        self._agents[context_id].interrupt()

  def try_acquire_context(self, context_id: str) -> bool:
    """原子地标记 context 为处理中；已 busy 返回 False"""
    with self._lock:
      if context_id in self._busy:
        return False
      self._busy.add(context_id)
      return True

  def release_context(self, context_id: str):
    """释放 context 的 busy 标记"""
    with self._lock:
      self._busy.discard(context_id)


def _apply_history_length(task: a2a_pb2.Task, history_length: int | None): # type: ignore
  """根据 history_length 裁剪 task.history"""
  if history_length is None or history_length < 0:
    return  # 不限制
  del task.history[:len(task.history) - history_length]


def task_status(state: int, message: a2a_pb2.Message = None) -> a2a_pb2.TaskStatus: # type: ignore
  status = a2a_pb2.TaskStatus(state=state) # type: ignore
  if message:
    status.message.CopyFrom(message)
  return status