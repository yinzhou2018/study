"""
A2A gRPC 交互式客户端：连接 a2a_server 进行多轮对话。

用法：
  python3 a2a_client.py
  python3 a2a_client.py --host 127.0.0.1 --port 50051

命令：/exit /clear /zone <zone> /help
会话内多轮共享 context_id；上一轮若 INPUT_REQUIRED，下一轮自动带 task_id 续接。
"""
import argparse
import uuid

import grpc
from google.protobuf.json_format import MessageToDict

from a2a import a2a_pb2, a2a_pb2_grpc
from config import AUDIO_ZONES, DEFAULT_ZONE


# TaskState 枚举值 → 可读名
_STATE_NAMES = {
    a2a_pb2.TaskState.TASK_STATE_SUBMITTED: "SUBMITTED",
    a2a_pb2.TaskState.TASK_STATE_WORKING: "WORKING",
    a2a_pb2.TaskState.TASK_STATE_COMPLETED: "COMPLETED",
    a2a_pb2.TaskState.TASK_STATE_FAILED: "FAILED",
    a2a_pb2.TaskState.TASK_STATE_CANCELED: "CANCELED",
    a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED: "INPUT_REQUIRED",
    a2a_pb2.TaskState.TASK_STATE_REJECTED: "REJECTED",
    a2a_pb2.TaskState.TASK_STATE_AUTH_REQUIRED: "AUTH_REQUIRED",
}

# 流末状态：携带 agent 回复、本轮结束（含可续接的 INPUT_REQUIRED）
_REPLY_STATES = {
    a2a_pb2.TaskState.TASK_STATE_COMPLETED,
    a2a_pb2.TaskState.TASK_STATE_FAILED,
    a2a_pb2.TaskState.TASK_STATE_CANCELED,
    a2a_pb2.TaskState.TASK_STATE_REJECTED,
    a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED,
    a2a_pb2.TaskState.TASK_STATE_AUTH_REQUIRED,
}


def _state_name(state):
  return _STATE_NAMES.get(state, str(state))


class A2AClientSession:
  """单会话状态：context_id + 上一轮任务（用于 INPUT_REQUIRED 续接）"""

  def __init__(self, zone=DEFAULT_ZONE):
    self.reset(zone)

  def reset(self, zone=DEFAULT_ZONE):
    self.context_id = str(uuid.uuid4())
    self.last_task_id = ""
    self.last_terminal = None
    self.zone = zone

  def should_continue_task(self):
    """上一轮 INPUT_REQUIRED 且有 task_id → 下一轮续接同一任务"""
    return (self.last_terminal == a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED
            and bool(self.last_task_id))


class A2AClient:
  """交互式 A2A 客户端：流式发送、实时打印事件、自动续接"""

  def __init__(self, stub):
    self.stub = stub
    self.session = A2AClientSession()

  def run(self):
    print(f"会话 context_id: {self.session.context_id}")
    print(f"音区: {self.session.zone}（/zone 切换）  输入 /help 查看命令")
    while True:
      try:
        line = input(self._prompt()).strip()
      except (EOFError, KeyboardInterrupt):
        print()
        break
      if not line:
        continue
      if line.startswith("/"):
        if self._handle_command(line):
          break
        continue
      self._send(line)

  def _prompt(self):
    ctx = self.session.context_id[:8]
    cont = " 续" if self.session.should_continue_task() else ""
    return f"[{ctx} {self.session.zone}{cont}] 你: "

  def _handle_command(self, line):
    parts = line.split(maxsplit=1)
    cmd, arg = parts[0], (parts[1] if len(parts) > 1 else "")
    if cmd == "/exit":
      return True
    if cmd == "/clear":
      self.session.reset(self.session.zone)
      print(f"已重置会话，新 context_id: {self.session.context_id}")
    elif cmd == "/zone":
      if arg not in AUDIO_ZONES:
        print(f"无效音区，可选: {', '.join(AUDIO_ZONES)}")
      else:
        self.session.zone = arg
        print(f"音区已切换: {arg}")
    elif cmd == "/help":
      print("/exit 退出 | /clear 重置会话 | /zone <zone> 切音区 | /help 帮助")
    else:
      print(f"未知命令: {cmd}（/help 查看）")
    return False

  def _send(self, text):
    """构建消息并发送；INPUT_REQUIRED 上一轮自动带 task_id 续接"""
    if self.session.should_continue_task():
      task_id, context_id = self.session.last_task_id, ""  # 续接：服务端推断 context
    else:
      task_id, context_id = "", self.session.context_id    # 新任务：同会话多轮
    msg = a2a_pb2.Message(
        message_id=str(uuid.uuid4()),
        task_id=task_id,
        context_id=context_id,
        role=a2a_pb2.Role.ROLE_USER,
        parts=[a2a_pb2.Part(text=text)],
    )
    msg.metadata["zone"] = self.session.zone
    try:
      terminal, reply, task_id = self._run_stream(
          a2a_pb2.SendMessageRequest(message=msg))
    except grpc.RpcError as e:
      print(f"  [错误] {e.code()}: {e.details()}")
      return
    self.session.last_task_id = task_id
    self.session.last_terminal = terminal
    self._print_result(terminal, reply)

  def _run_stream(self, req):
    """消费流事件，返回 (terminal_state, reply, task_id)；过程事件实时打印"""
    terminal, reply, task_id = None, "", ""
    for ev in self.stub.SendStreamingMessage(req):
      payload = ev.WhichOneof("payload")
      if payload == "task":
        task_id = ev.task.id
        print(f"  [Task {task_id[:8]}] state={_state_name(ev.task.status.state)}")
      elif payload == "status_update":
        su = ev.status_update
        if su.task_id:
          task_id = su.task_id
        if su.status.state in _REPLY_STATES:
          terminal = su.status.state
          reply = "".join(p.text or "" for p in su.status.message.parts)
        else:
          print(f"  [状态] {_state_name(su.status.state)}")
      elif payload == "artifact_update":
        self._print_artifact(ev.artifact_update)
    return terminal, reply, task_id

  def _print_artifact(self, au):
    """打印过程事件增量（reasoning/reply/tool_call/tool_result）"""
    name = au.artifact.name
    for p in au.artifact.parts:
      which = p.WhichOneof("content")
      if which == "text":
        print(f"  [{name}] {p.text}")
      elif which == "data":
        print(f"  [{name}] {MessageToDict(p.data)}")

  def _print_result(self, terminal, reply):
    if terminal == a2a_pb2.TaskState.TASK_STATE_COMPLETED:
      print(f"  助手: {reply}")
    elif terminal == a2a_pb2.TaskState.TASK_STATE_INPUT_REQUIRED:
      print(f"  助手(需补充): {reply}")
      print("  ↑ 请补充输入后回车，将自动续接该任务")
    elif terminal == a2a_pb2.TaskState.TASK_STATE_FAILED:
      print(f"  [失败] {reply}")
    elif terminal is None:
      print("  [未收到终态]")
    else:
      print(f"  [{_state_name(terminal)}] {reply}")


def main():
  parser = argparse.ArgumentParser(description="A2A gRPC 交互式客户端")
  parser.add_argument("--host", default="127.0.0.1", help="a2a_server 地址")
  parser.add_argument("--port", type=int, default=50051, help="a2a_server 端口")
  args = parser.parse_args()

  address = f"{args.host}:{args.port}"
  channel = grpc.insecure_channel(address)
  stub = a2a_pb2_grpc.A2AServiceStub(channel)
  # 连通性检查：拉取 AgentCard
  try:
    card = stub.GetExtendedAgentCard(
        a2a_pb2.GetExtendedAgentCardRequest(tenant="default"))
  except grpc.RpcError as e:
    print(f"无法连接 a2a_server ({address}): {e.code()}")
    print("请先启动: python3 main.py --a2a")
    return
  print(f"已连接: {card.name} v{card.version}（{len(card.skills)} skills）@ {address}")
  A2AClient(stub).run()
  channel.close()


if __name__ == "__main__":
  main()
