"""
A2A gRPC 服务器：实现 A2AService 协议，接收外部 Agent 请求。

支持 RPC：
  - GetExtendedAgentCard  动态构建 AgentCard
  - SendMessage            同步 chat → Task(COMPLETED/FAILED)
  - SendStreamingMessage   chat_stream → WORKING → Message → COMPLETED
  - GetTask                查询任务状态与历史
  - CancelTask             标记为 CANCELED 并触发 agent.interrupt()
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

  # ---------- SendMessage ----------

  def SendMessage(self, request, context):
    msg = request.message
    text = extract_text(msg)
    if not text:
      context.set_code(grpc.StatusCode.INVALID_ARGUMENT)
      context.set_details("message text is empty")
      return a2a_pb2.SendMessageResponse() # type: ignore

    context_id = msg.context_id or str(uuid.uuid4())
    task_id = msg.task_id or str(uuid.uuid4())
    zone = extract_zone(msg)

    # 构建任务
    task = a2a_pb2.Task( # type: ignore
        id=task_id,
        context_id=context_id,
        status=a2a_pb2.TaskStatus(state=a2a_pb2.TaskState.TASK_STATE_SUBMITTED), # type: ignore
    )
    task.history.add(
        message_id=msg.message_id or str(uuid.uuid4()),
        role=msg.role,
        parts=msg.parts,
    )
    self.store.put_task(task)

    # 执行对话
    agent = self.store.get_or_create_agent(context_id)
    self.store.touch_agent(context_id)
    try:
      reply = agent.chat(text, verbose=False, zone_id=zone)
    except Exception as e:
      task.status.state = a2a_pb2.TaskState.TASK_STATE_FAILED # type: ignore
      task.status.message.CopyFrom(
          build_agent_message(f"执行失败：{e}", task_id, context_id)
      )
      self.store.put_task(task)
      return a2a_pb2.SendMessageResponse(task=task) # type: ignore

    # 记录回复
    reply_msg = build_agent_message(reply, task_id, context_id)
    task.history.add(
        message_id=reply_msg.message_id,
        role=reply_msg.role,
        parts=reply_msg.parts,
    )
    task.status.state = a2a_pb2.TaskState.TASK_STATE_COMPLETED # type: ignore
    task.status.message.CopyFrom(reply_msg)
    self.store.put_task(task)
    return a2a_pb2.SendMessageResponse(task=task) # type: ignore

  # ---------- SendStreamingMessage ----------

  def SendStreamingMessage(self, request, context):
    msg = request.message
    text = extract_text(msg)
    if not text:
      context.set_code(grpc.StatusCode.INVALID_ARGUMENT)
      context.set_details("message text is empty")
      return

    context_id = msg.context_id or str(uuid.uuid4())
    task_id = msg.task_id or str(uuid.uuid4())
    zone = extract_zone(msg)

    task = a2a_pb2.Task( # type: ignore
        id=task_id,
        context_id=context_id,
        status=a2a_pb2.TaskStatus(state=a2a_pb2.TaskState.TASK_STATE_WORKING), # type: ignore
    )
    task.history.add(
        message_id=msg.message_id or str(uuid.uuid4()),
        role=msg.role,
        parts=msg.parts,
    )
    self.store.put_task(task)

    # 1. 通知 WORKING
    yield a2a_pb2.StreamResponse( # type: ignore
        status_update=a2a_pb2.TaskStatusUpdateEvent( # type: ignore
            task_id=task_id,
            context_id=context_id,
            status=a2a_pb2.TaskStatus(state=a2a_pb2.TaskState.TASK_STATE_WORKING), # type: ignore
        )
    )

    # 2. 执行对话，收集回复
    agent = self.store.get_or_create_agent(context_id)
    self.store.touch_agent(context_id)
    try:
      reply = agent.chat_stream(text, zone_id=zone)
    except InterruptedError:
      reply = "操作已打断。"
    except Exception as e:
      reply = f"执行失败：{e}"

    reply_msg = build_agent_message(reply, task_id, context_id)
    task.history.add(
        message_id=reply_msg.message_id,
        role=reply_msg.role,
        parts=reply_msg.parts,
    )
    task.status.state = a2a_pb2.TaskState.TASK_STATE_COMPLETED # type: ignore
    task.status.message.CopyFrom(reply_msg)
    self.store.put_task(task)

    # 3. 发送最终回复
    yield a2a_pb2.StreamResponse(message=reply_msg) # type: ignore

    # 4. 发送 COMPLETED 状态
    yield a2a_pb2.StreamResponse( # type: ignore
        status_update=a2a_pb2.TaskStatusUpdateEvent( # type: ignore
            task_id=task_id,
            context_id=context_id,
            status=a2a_pb2.TaskStatus(state=a2a_pb2.TaskState.TASK_STATE_COMPLETED), # type: ignore
        )
    )

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

    terminal = {
        a2a_pb2.TaskState.TASK_STATE_COMPLETED, # type: ignore
        a2a_pb2.TaskState.TASK_STATE_FAILED, # type: ignore
        a2a_pb2.TaskState.TASK_STATE_CANCELED, # type: ignore
        a2a_pb2.TaskState.TASK_STATE_REJECTED, # type: ignore
    }
    if task.status.state in terminal:
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