import json

from config import build_system_prompt
from llm_client import MockLLMClient
from tool_gateway import ToolGateway
from toolset_manager import ToolsetManager


class CockpitAgent:
  def __init__(self, llm_client=None):
    self.llm = llm_client or MockLLMClient()
    self.toolset_manager = ToolsetManager()
    self.tool_gateway = ToolGateway(self.toolset_manager)
    # 初始化消息：System Prompt(含动态工具集列表) + 空对话
    self.messages = [
        {"role": "system", "content": build_system_prompt(self.toolset_manager.get_toolset_listing())}
    ]
    self.max_turns = 8  # 单轮用户输入最多执行8轮工具调用，防止死循环

  def chat(self, user_query: str, verbose: bool = True) -> str:
    """用户输入一句话，执行完整的工具调用链路，返回最终回复"""
    self.messages.append({"role": "user", "content": user_query})

    for turn in range(self.max_turns):
      # 动态获取当前工具列表
      current_tools = self.toolset_manager.get_current_tools()

      if verbose:
        print(f"\n--- 第{turn+1}轮模型调用 ---")
        print(f"当前激活工具集: {self.toolset_manager.get_active_toolset_ids()}")
        print(f"当前可用工具数: {len(current_tools)}")

      # 调用大模型
      response = self.llm.chat(
          messages=self.messages,
          tools=current_tools,
          temperature=0.1
      )

      msg = response["choices"][0]["message"]
      self.messages.append(msg)

      # 没有工具调用，任务结束
      if not msg.get("tool_calls"):
        if verbose:
          print(f"模型最终回复: {msg['content']}")
        return msg["content"]

      # 处理每一个工具调用
      for tool_call in msg["tool_calls"]:
        tool_name = tool_call["function"]["name"]
        try:
          arguments = json.loads(tool_call["function"]["arguments"])
        except:
          arguments = {}

        if verbose:
          print(f"调用工具: {tool_name}, 参数: {arguments}")

        # 特殊处理：加载工具集
        if tool_name == "load_toolsets":
          toolset_ids = arguments.get("toolset_ids", [])
          result = self.toolset_manager.load_toolsets(toolset_ids)
        elif tool_name == "list_active_toolsets":
          result = self.tool_gateway.execute(tool_name, arguments)
        else:
          # 普通业务工具，走网关执行
          result = self.tool_gateway.execute(tool_name, arguments)

        # 回填工具结果
        self.messages.append({
            "role": "tool",
            "tool_call_id": tool_call["id"],
            "content": json.dumps(result, ensure_ascii=False)
        })

        if verbose:
          print(f"工具返回: {result}")

    return "操作执行完毕。"

  def compress_history(self):
    """历史压缩：将工具执行细节压缩为摘要，保留核心语义"""
    if len(self.messages) <= 6:
      return

    # 提取System + 最近3轮用户/助手回复，中间的工具调用压缩为摘要
    system_msg = self.messages[0]
    recent_msgs = self.messages[-4:]

    # 生成执行摘要
    summary_lines = []
    for m in self.messages[1:-4]:
      if m["role"] == "tool":
        try:
          data = json.loads(m["content"])
          if data.get("status") == "success" and "message" in data:
            summary_lines.append(data["message"])
        except:
          pass

    summary_msg = {
        "role": "system",
        "content": "【执行摘要】此前已完成操作：" + "；".join(summary_lines)
    }

    self.messages = [system_msg, summary_msg] + recent_msgs
