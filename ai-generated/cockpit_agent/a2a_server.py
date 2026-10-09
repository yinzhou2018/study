"""
A2A gRPC 服务器：实现 A2AService 协议，接收外部 Agent 请求。

支持 RPC：
  - GetExtendedAgentCard  动态构建 AgentCard
  - SendMessage            解析 task_id/context_id 后同步 chat → Task(COMPLETED/INPUT_REQUIRED/FAILED)
  - SendStreamingMessage   解析 task_id/context_id → 初始 Task → 终态 TaskStatusUpdateEvent
  - GetTask                查询任务状态与历史
  - CancelTask             标记为 CANCELED 并触发 agent.interrupt()

流式事件规范：新任务首事件为 Task 对象；续接已有任务首事件为 WORKING
TaskStatusUpdateEvent；解析失败与并发拒绝均为单一 FAILED TaskStatusUpdateEvent；
agent 回复经终态事件 status.message 投递，不发独立 Message 事件。

task_id/context_id 解析（遵循 proto 语义）：
  - 无 task_id：新建 Task（context_id 复用传入值或新建）
  - 仅 task_id：命中已有非终态任务并推断 context_id，否则返回 FAILED（不写入 store）
  - 两者都传：额外校验 context_id 与任务绑定一致

INPUT_REQUIRED：模型调用 request_user_input 请求澄清时任务转该状态，
客户端用 task_id 续接补充输入后转 COMPLETED。
"""
import time
import uuid
from concurrent import futures

import grpc
from google.protobuf import empty_pb2

from a2a import a2a_pb2, a2a_pb2_grpc
from a2a_agent import (
    TaskStore,
    extract_text,
    extract_zone,
    build_agent_message,
    task_status,
    _apply_history_length,
)
from config import DEFAULT_ZONE


# 任务终态集合：处于这些状态的任务生命周期已结束，不可继续接收消息
_TERMINAL_STATES = frozenset({
    a2a_pb2.TaskState.TASK_STATE_COMPLETED, # type: ignore
    a2a_pb2.TaskState.TASK_STATE_FAILED, # type: ignore
    a2a_pb2.TaskState.TASK_STATE_CANCELED, # type: ignore
    a2a_pb2.TaskState.TASK_STATE_REJECTED, # type: ignore
})


class A2AServiceServicer(a2a_pb2_grpc.A2AServiceServicer):
  """A2AService 协议实现"""

  def __init__(self, store: TaskStore, address: str):
    self.store = store
    self.address = address
    # AgentCard 构建依赖（在启动时实例化，避免每次请求重新创建）
    self._agent_card = self._build_agent_card()

  # ---------- AgentCard ----------

  def GetExtendedAgentCard(self, request, context):
    return self._agent_card

  def _build_agent_card(self) -> a2a_pb2.AgentCard: # type: ignore
    from skill_manager import SkillManager
    from toolset_manager import ToolsetManager

    tm = ToolsetManager()
    sm = SkillManager()

    # 从工具集派生 AgentSkill
    skills = [
        a2a_pb2.AgentSkill( # type: ignore
            id=tid,
            name=cfg["name"],
            description=cfg["description"],
        )
        for tid, cfg in tm.all_toolsets_config.items()
    ]
    # 从已发现技能派生 AgentSkill
    for name in sm.get_all_skill_names():
      skill = sm._skills[name]
      skills.append(a2a_pb2.AgentSkill( # type: ignore
          id=name,
          name=name,
          description=skill["description"],
      ))

    host, _, port_str = self.address.rpartition(":")
    port = port_str or "50051"
    url = f"http://{host}:{port}" if host else f"http://localhost:{port}"

    return a2a_pb2.AgentCard( # type: ignore
        name="智能座舱助手",
        description="智能座舱车载助手，提供车辆控制、信息查询、娱乐导航等对话服务。",
        version="1.0.0",
        supported_interfaces=[
            a2a_pb2.AgentInterface( # type: ignore
                url=url,
                protocol_binding="A2A/GRPC",
                protocol_version="0.3.0",
            )
        ],
        capabilities=a2a_pb2.AgentCapabilities(streaming=True), # type: ignore
        default_input_modes=["text/plain"],
        default_output_modes=["text/plain"],
        skills=skills,
    )

  # ---------- task_id / context_id 解析 ----------

  def _resolve_task(self, msg):
    """按 proto 语义解析 task_id/context_id，返回 (task, context_id, error)。

    - 无 task_id：新建任务（context_id 用传入或新建）
    - 仅 task_id：查找已有任务并推断 context_id
    - 两者都传：还需校验 context_id 与任务绑定一致
    错误时 task 为 None，context_id 为失败响应应使用的值。
    """
    task_id = msg.task_id
    context_id = msg.context_id

    # 无 task_id：新建任务，context 复用传入值或新生成
    if not task_id:
      new_context = context_id or str(uuid.uuid4())
      task = a2a_pb2.Task( # type: ignore
          id=str(uuid.uuid4()),
          context_id=new_context,
          status=a2a_pb2.TaskStatus( # type: ignore
              state=a2a_pb2.TaskState.TASK_STATE_SUBMITTED), # type: ignore
      )
      return task, new_context, None

    # 有 task_id：必须命中未结束的已有任务
    existing = self.store.get_task(task_id)
    if not existing:
      return None, context_id, f"任务不存在：{task_id}"
    if context_id and existing.context_id != context_id:
      return None, existing.context_id, (
          f"context_id 不匹配：任务绑定 {existing.context_id}，"
          f"请求为 {context_id}")
    if existing.status.state in _TERMINAL_STATES:
      return None, existing.context_id, "任务已结束，无法继续"
    return existing, existing.context_id, None

  def _failed_task(self, task_id, context_id, error):
    """构造解析失败的响应 Task（FAILED，不写入 store，不影响已有任务）"""
    return a2a_pb2.Task( # type: ignore
        id=task_id,
        context_id=context_id,
        status=task_status(
            a2a_pb2.TaskState.TASK_STATE_FAILED, # type: ignore
            build_agent_message(error, task_id, context_id)),
    )

  # ---------- 任务执行辅助 ----------

  def _append_user_message(self, task, msg):
    """将用户消息追加到任务历史，置为 WORKING 并写入 store"""
    task.history.add(
        message_id=msg.message_id or str(uuid.uuid4()),
        role=msg.role,
        parts=msg.parts,
    )
    task.status.state = a2a_pb2.TaskState.TASK_STATE_WORKING # type: ignore
    self.store.put_task(task)

  def _record_reply(self, task, context_id, reply, agent):
    """记录 agent 回复到任务历史，按 needs_input 置 COMPLETED/INPUT_REQUIRED"""
    reply_msg = build_agent_message(reply, task.id, context_id)
    task.history.add(
        message_id=reply_msg.message_id,
        role=reply_msg.role,
        parts=reply_msg.parts,
    )
    # 模型请求补充输入 → INPUT_REQUIRED（等待客户端用 task_id 续接）
    if getattr(agent, "needs_input", False):
      task.status.state = a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED # type: ignore
    else:
      task.status.state = a2a_pb2.TaskState.TASK_STATE_COMPLETED # type: ignore
    task.status.message.CopyFrom(reply_msg)
    self.store.put_task(task)
    return reply_msg

  def _fail_running(self, task, context_id, error):
    """运行期失败：任务置为 FAILED 并写入 store"""
    task.status.state = a2a_pb2.TaskState.TASK_STATE_FAILED # type: ignore
    task.status.message.CopyFrom(
        build_agent_message(error, task.id, context_id)
    )
    self.store.put_task(task)

  def _status_event(self, task_id, context_id, status):
    """构造任务状态更新流事件"""
    return a2a_pb2.StreamResponse( # type: ignore
        status_update=a2a_pb2.TaskStatusUpdateEvent( # type: ignore
            task_id=task_id,
            context_id=context_id,
            status=status,
        )
    )

  def _stream_failure(self, task_id, context_id, error):
    """解析失败事件序列：仅终态 TaskStatusUpdateEvent（不写 store，不发 Task 事件）"""
    failed_status = task_status(
        a2a_pb2.TaskState.TASK_STATE_FAILED, # type: ignore
        build_agent_message(error, task_id, context_id))
    yield self._status_event(task_id, context_id, failed_status)

  def _busy_failure(self, task_id, context_id):
    """并发拒绝事件：单一 FAILED TaskStatusUpdateEvent"""
    busy_msg = build_agent_message("任务处理中，请稍后重试", task_id, context_id)
    yield self._status_event(
        task_id, context_id,
        task_status(a2a_pb2.TaskState.TASK_STATE_FAILED, busy_msg)) # type: ignore

  # ---------- SendMessage ----------

  def SendMessage(self, request, context):
    msg = request.message
    text = extract_text(msg)
    if not text:
      context.set_code(grpc.StatusCode.INVALID_ARGUMENT)
      context.set_details("message text is empty")
      return a2a_pb2.SendMessageResponse() # type: ignore

    # 解析/校验 task_id 与 context_id
    task, context_id, error = self._resolve_task(msg)
    if error:
      return a2a_pb2.SendMessageResponse( # type: ignore
          task=self._failed_task(msg.task_id, context_id, error))

    # 并发保护：同一 context 同时只处理一个请求
    if not self.store.try_acquire_context(context_id):
      return a2a_pb2.SendMessageResponse( # type: ignore
          task=self._failed_task(
              task.id, context_id, "任务处理中，请稍后重试"))

    try:
      return self._run_send_message(task, context_id, msg, text)
    finally:
      self.store.release_context(context_id)

  def _run_send_message(self, task, context_id, msg, text):
    """执行对话并回填回复（调用前已持有 context busy 锁）"""
    zone = extract_zone(msg)
    self._append_user_message(task, msg)
    agent = self.store.get_or_create_agent(context_id)
    self.store.touch_agent(context_id)
    try:
      reply = agent.chat(text, verbose=False, zone_id=zone)
    except Exception as e:
      self._fail_running(task, context_id, f"执行失败：{e}")
      return a2a_pb2.SendMessageResponse(task=task) # type: ignore

    self._record_reply(task, context_id, reply, agent)
    return a2a_pb2.SendMessageResponse(task=task) # type: ignore

  # ---------- SendStreamingMessage ----------

  def SendStreamingMessage(self, request, context):
    msg = request.message
    text = extract_text(msg)
    if not text:
      context.set_code(grpc.StatusCode.INVALID_ARGUMENT)
      context.set_details("message text is empty")
      return

    # 解析/校验 task_id 与 context_id
    task, context_id, error = self._resolve_task(msg)
    if error:
      yield from self._stream_failure(msg.task_id, context_id, error)
      return

    # 并发保护：同一 context 同时只处理一个请求
    if not self.store.try_acquire_context(context_id):
      yield from self._busy_failure(task.id, context_id) # type: ignore
      return

    try:
      yield from self._run_stream(task, context_id, msg, text)
    finally:
      self.store.release_context(context_id)

  def _run_stream(self, task, context_id, msg, text):
    """执行对话并产出流事件（调用前已持有 context busy 锁）"""
    zone = extract_zone(msg)
    self._append_user_message(task, msg)

    # 初始回复：新任务发 Task 对象，续接已有任务发 WORKING status_update
    if not msg.task_id:
      yield a2a_pb2.StreamResponse(task=task) # type: ignore
    else:
      yield self._status_event(task.id, context_id, task.status)

    # 执行对话；异常置 FAILED，正常按 needs_input 置终态
    agent = self.store.get_or_create_agent(context_id)
    self.store.touch_agent(context_id)
    try:
      reply = agent.chat_stream(text, zone_id=zone)
    except InterruptedError:
      self._fail_running(task, context_id, "操作已打断。")
    except Exception as e:
      self._fail_running(task, context_id, f"执行失败：{e}")
    else:
      self._record_reply(task, context_id, reply, agent)

    # 终态事件：TaskStatusUpdateEvent（status.message 携带回复）
    yield self._status_event(task.id, context_id, task.status) # type: ignore

  # ---------- GetTask ----------

  def GetTask(self, request, context):
    task = self.store.get_task(request.id)
    if not task:
      context.set_code(grpc.StatusCode.NOT_FOUND)
      context.set_details(f"Task {request.id} not found")
      return a2a_pb2.Task() # type: ignore

    # history_length: optional int32，HasField 检测是否设置
    if request.HasField("history_length") and request.history_length >= 0:
      _apply_history_length(task, request.history_length)
    return task

  # ---------- CancelTask ----------

  def CancelTask(self, request, context):
    task = self.store.get_task(request.id)
    if not task:
      context.set_code(grpc.StatusCode.NOT_FOUND)
      context.set_details(f"Task {request.id} not found")
      return a2a_pb2.Task() # type: ignore

    if task.status.state in _TERMINAL_STATES:
      return task  # 已终态，无需操作

    task.status.state = a2a_pb2.TaskState.TASK_STATE_CANCELED # type: ignore
    self.store.put_task(task)
    # 尝试打断正在运行的 agent
    if task.context_id:
      self.store.cancel_running_task(task.context_id)
    return task

  # ---------- 未实现的 RPC（原型够用）----------

  def ListTasks(self, request, context):
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    return a2a_pb2.ListTasksResponse() # type: ignore

  def SubscribeToTask(self, request, context):
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    yield  # must be a generator

  def CreateTaskPushNotificationConfig(self, request, context):
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    return a2a_pb2.TaskPushNotificationConfig() # type: ignore

  def GetTaskPushNotificationConfig(self, request, context):
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    return a2a_pb2.TaskPushNotificationConfig() # type: ignore

  def ListTaskPushNotificationConfigs(self, request, context):
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    return a2a_pb2.ListTaskPushNotificationConfigsResponse() # type: ignore

  def DeleteTaskPushNotificationConfig(self, request, context):
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    return empty_pb2.Empty()


# ---------------------------------------------------------------------------
# 服务器启动
# ---------------------------------------------------------------------------

def serve(
    host: str = "0.0.0.0",
    port: int = 50051,
    max_workers: int = 10,
    llm_client=None,
    skills_dir=None,
):
  """启动 A2A gRPC 服务器（同步，block）"""
  address = f"{host}:{port}"
  store = TaskStore(skills_dir=skills_dir)
  store.set_llm(llm_client)
  servicer = A2AServiceServicer(store, address)
  server = grpc.server(futures.ThreadPoolExecutor(max_workers=max_workers))
  a2a_pb2_grpc.add_A2AServiceServicer_to_server(servicer, server)
  server.add_insecure_port(f"[::]:{port}")
  server.start()
  print(f"✓ A2A gRPC 服务器启动: {address}")
  print(f"  AgentCard: http://{host}:{port}/.well-known/agent-card  (gRPC)")
  print(f"  Max concurrent contexts: {store.max_contexts}")
  print(f"  Press Ctrl+C to stop")
  server.wait_for_termination()