import json

from config import DEFAULT_USER_ID, DEFAULT_ZONE, build_system_prompt
from llm_client import MockLLMClient, OpenAICompatibleLLM
from llm_config import DEFAULT_EFFORT, DEFAULT_PROVIDER_ID, PROVIDERS
from skill_manager import SkillManager
from tool_gateway import ToolGateway
from toolset_manager import ToolsetManager
from zone_context import parse_user_tag, tag_user_message


class CockpitAgent:
  def __init__(self, llm_client=None, provider_id=None, skills_dir=None):
    self.llm = llm_client or MockLLMClient()
    self.toolset_manager = ToolsetManager()
    self.skill_manager = SkillManager(skills_dir=skills_dir)
    self.tool_gateway = ToolGateway(self.toolset_manager)
    # 初始化消息：System Prompt(含动态工具集列表 + 技能清单) + 空对话
    self.messages = [
        {"role": "system", "content": build_system_prompt(
            self.toolset_manager.get_toolset_listing(),
            self.skill_manager.get_skill_listing(),
        )}
    ]
    self.max_turns = 8  # 单轮用户输入最多执行8轮工具调用，防止死循环
    self._interrupted = False
    self.effort = DEFAULT_EFFORT
    self.provider_id = provider_id or DEFAULT_PROVIDER_ID

  def set_provider(self, provider_id):
    """运行期切换 LLM 供应商，保留对话历史"""
    if provider_id not in PROVIDERS:
      raise ValueError(
          f"未知的供应商: {provider_id}, 可选值: {', '.join(PROVIDERS.keys())}"
      )
    self.provider_id = provider_id
    profile = PROVIDERS[provider_id]
    # OpenAICompatibleLLM 支持运行期重配置；MockLLMClient 等无 configure 方法时仅更新 id
    if hasattr(self.llm, "configure"):
      self.llm.configure( # type: ignore
          profile["base_url"],
          profile["api_key"],
          profile["model"],
          provider_id=provider_id,
      )

  def interrupt(self):
    """设置打断标志，由REPL的Esc监听器调用"""
    self._interrupted = True

  def _check_interrupt(self):
    if self._interrupted:
      raise InterruptedError("用户打断")

  def chat(self, user_query: str, verbose: bool = True,
           zone_id: str = DEFAULT_ZONE, user_id: str = DEFAULT_USER_ID) -> str:
    """用户输入一句话，执行完整的工具调用链路，返回最终回复"""
    self.messages.append({
        "role": "user",
        "content": tag_user_message(user_query, zone_id, user_id)
    })

    for turn in range(self.max_turns):
      # 动态获取当前完整工具列表（系统工具 + 业务工具 + 技能工具）
      current_tools = self._get_current_tools()

      if verbose:
        print(f"\n--- 第{turn+1}轮模型调用 ---")
        print(f"当前激活工具集: {self.toolset_manager.get_active_toolset_ids()}")
        print(f"当前可用工具数: {len(current_tools)}")

      # 调用大模型
      response = self.llm.chat(
          messages=self.messages,
          tools=current_tools,
          temperature=0.1,
          effort=self.effort
      )

      msg = response["choices"][0]["message"]
      self.messages.append(msg)

      # 没有工具调用，任务结束
      if not msg.get("tool_calls"):
        if verbose:
          print(f"模型最终回复: {msg['content']}")
        return msg.get("content") or ""

      # 处理每一个工具调用
      for tool_call in msg["tool_calls"]:
        tool_name = tool_call["function"]["name"]
        try:
          arguments = json.loads(tool_call["function"]["arguments"])
        except Exception:
          arguments = {}

        if verbose:
          print(f"调用工具: {tool_name}, 参数: {arguments}")

        # 特殊处理：加载技能（技能内容随工具结果返回，无 LRU 淘汰）
        if tool_name == "load_skills":
          result = self.skill_manager.load_skills(arguments.get("skill_names", []))
        elif tool_name == "load_toolsets":
          toolset_ids = arguments.get("toolset_ids", [])
          result = self.toolset_manager.load_toolsets(toolset_ids)
        elif tool_name == "list_active_toolsets":
          result = self.tool_gateway.execute(tool_name, arguments, requesting_zone=zone_id)
        else:
          # 普通业务工具，走网关执行
          result = self.tool_gateway.execute(tool_name, arguments, requesting_zone=zone_id)

        # 回填工具结果
        self.messages.append({
            "role": "tool",
            "tool_call_id": tool_call["id"],
            "content": json.dumps(result, ensure_ascii=False)
        })

        if verbose:
          print(f"工具返回: {result}")

    return "操作执行完毕。"

  def chat_stream(self, user_query, on_content=None, on_tool_call=None,
                  on_reasoning=None, on_tool_result=None, on_call_llm=None, on_done=None,
                  zone_id: str = DEFAULT_ZONE, user_id: str = DEFAULT_USER_ID):
    """流式对话：逐token回调思考内容和回复，工具调用实时回调，支持打断"""
    self._interrupted = False
    self.messages.append({
        "role": "user",
        "content": tag_user_message(user_query, zone_id, user_id)
    })

    try:
      for turn in range(self.max_turns):
        self._check_interrupt()

        if on_call_llm:
          on_call_llm(turn + 1, True)

        msg = None

        current_tools = self._get_current_tools()
        for event in self.llm.chat_stream(
            messages=self.messages,
            tools=current_tools,
            temperature=0.1,
            effort=self.effort
        ):
          self._check_interrupt()
          if event["type"] == "reasoning":
            if on_reasoning:
              on_reasoning(event["text"])
          elif event["type"] == "content":
            if on_content:
              on_content(event["text"])
          elif event["type"] == "done":
            msg = event["message"]

        if on_call_llm:
          on_call_llm(turn + 1, False)

        if msg is None:
          raise RuntimeError("LLM流式响应缺少done事件")

        self.messages.append(msg)  # type: ignore

        # 没有工具调用，任务结束
        if not msg.get("tool_calls"):  # type: ignore
          final_reply = msg.get("content") or ""  # type: ignore
          if on_done:
            on_done(final_reply)
          return final_reply

        # 处理每一个工具调用
        for tool_call in msg["tool_calls"]:  # type: ignore
          self._check_interrupt()

          tool_name = tool_call["function"]["name"]  # type: ignore
          try:
            arguments = json.loads(tool_call["function"]["arguments"])  # type: ignore
          except Exception:
            arguments = {}

          if on_tool_call:
            on_tool_call(tool_name, arguments)

          if tool_name == "load_skills":
            result = self.skill_manager.load_skills(arguments.get("skill_names", []))
          elif tool_name == "load_toolsets":
            toolset_ids = arguments.get("toolset_ids", [])
            result = self.toolset_manager.load_toolsets(toolset_ids)
          else:
            result = self.tool_gateway.execute(tool_name, arguments, requesting_zone=zone_id)

          self.messages.append({
              "role": "tool",
              "tool_call_id": tool_call["id"],  # type: ignore
              "content": json.dumps(result, ensure_ascii=False)
          })

          if on_tool_result:
            on_tool_result(tool_name, result)

      final_reply = "操作执行完毕。"
      if on_done:
        on_done(final_reply)
      return final_reply
    except InterruptedError:
      raise

  def compress_history(self):
    """历史压缩：将工具执行细节压缩为摘要，保留核心语义"""
    if len(self.messages) <= 6:
      return

    # 提取System + 最近3轮用户/助手回复，中间的工具调用压缩为摘要
    system_msg = self.messages[0]
    recent_msgs = self.messages[-4:]

    # 生成执行摘要
    summary_lines = []
    current_zone = "unknown"
    for m in self.messages[1:-4]:
      if m["role"] == "user":
        zone, _, _ = parse_user_tag(str(m.get("content", "")))
        if zone:
          current_zone = zone
      elif m["role"] == "tool":
        try:
          data = json.loads(m["content"])
          if data.get("status") == "success" and "message" in data:
            summary_lines.append(f"[{current_zone}] {data['message']}")
        except Exception:
          pass

    summary_msg = {
        "role": "system",
        "content": "【执行摘要】此前已完成操作：" + "；".join(summary_lines)
    }

    self.messages = [system_msg, summary_msg] + recent_msgs

  def _get_current_tools(self) -> list:
    """组装完整工具列表：系统工具 + 已激活业务工具 + 技能工具"""
    tools = self.toolset_manager.get_current_tools()
    # 追加 load_skills 系统工具（enum 为当前已发现的所有技能名）
    skill_names = self.skill_manager.get_all_skill_names()
    if skill_names:
      tools.append(self.skill_manager.get_system_tool(skill_names))
    return tools

  def reload_skills(self):
    """重新发现技能并刷新 System Prompt（供 REPL /reload 调用）"""
    self.skill_manager.discover_skills()
    self.messages[0] = {
        "role": "system",
        "content": build_system_prompt(
            self.toolset_manager.get_toolset_listing(),
            self.skill_manager.get_skill_listing(),
        )
    }
