import json
import time
import types

try:
  import requests
except ModuleNotFoundError:
  # 测试环境可能未安装 requests；此处占位以便 monkeypatch llm_client.requests.post
  requests = types.SimpleNamespace(post=None)

from llm_config import build_effort_payload


class MockLLMClient:
  """模拟LLM客户端，按预设逻辑返回工具调用，用于本地跑通全流程"""

  def __init__(self):
    pass  # 无状态

  def chat(self, messages, tools, temperature=0.1, effort=None):
    # 提取用户最后一句话
    user_msg = ""
    for m in reversed(messages):
      if m["role"] == "user":
        user_msg = m["content"]
        break

    # 判断当前是否已加载工具集
    tool_names = [t["function"]["name"] for t in tools]
    has_biz_tools = len([n for n in tool_names if n not in ["load_toolsets", "list_active_toolsets"]]) > 0

    # ========== 模拟逻辑：未加载业务工具时，先加载工具集 ==========
    if not has_biz_tools:
      load_ids = []
      if any(k in user_msg for k in ["闷", "通风", "开窗", "天窗"]):
        load_ids.append("toolset_quick_ventilation")
      if any(k in user_msg for k in ["音乐", "歌", "音量", "电台"]):
        load_ids.append("toolset_music_play")
      if any(k in user_msg for k in ["冷", "热", "温度", "空调"]):
        load_ids.append("toolset_temperature_control")
      if any(k in user_msg for k in ["续航", "胎压", "车况", "跑多远"]):
        load_ids.append("toolset_car_status_query")

      # 最多取2个
      load_ids = load_ids[:2]

      if load_ids:
        return self._build_function_response(
            "load_toolsets",
            {"toolset_ids": load_ids}
        )

    # ========== 模拟逻辑：已加载通风工具集 ==========
    if "sys_query_vehicle_speed" in tool_names:
      # 第一步：查车速
      has_speed_result = any(
          m["role"] == "tool" and "speed" in m["content"]
          for m in messages
      )
      if not has_speed_result:
        return self._build_function_response("sys_query_vehicle_speed", {})

      # 第二步：执行通风控制
      has_window_result = any(
          m["role"] == "tool" and "车窗" in m["content"]
          for m in messages
      )
      if not has_window_result:
        return self._build_function_response("ctrl_window", {"position": "front_left", "openness": 25})

      has_sunroof_result = any(
          m["role"] == "tool" and "天窗" in m["content"]
          for m in messages
      )
      if not has_sunroof_result:
        return self._build_function_response("ctrl_sunroof_tilt", {"status": "on"})

      has_cir_result = any(
          m["role"] == "tool" and "循环" in m["content"]
          for m in messages
      )
      if not has_cir_result:
        return self._build_function_response("ctrl_ac_circulation", {"mode": "outer"})

    # ========== 模拟逻辑：已加载音乐工具集 ==========
    if "ctrl_music_play" in tool_names:
      has_music_result = any(
          m["role"] == "tool" and "播放" in m["content"]
          for m in messages
      )
      if not has_music_result:
        return self._build_function_response("ctrl_music_play", {"category": "light"})

    # ========== 模拟逻辑：已加载温度工具集 ==========
    if "ctrl_ac_temperature" in tool_names:
      has_ac_result = any(
          m["role"] == "tool" and "空调温度" in m["content"]
          for m in messages
      )
      if not has_ac_result:
        return self._build_function_response("ctrl_ac_temperature", {"temperature": 25})

    # ========== 模拟逻辑：已加载车况查询工具集 ==========
    if "query_range_mileage" in tool_names:
      has_range_result = any(
          m["role"] == "tool" and "range_km" in m["content"]
          for m in messages
      )
      if not has_range_result:
        return self._build_function_response("query_range_mileage", {})

    # 所有工具执行完毕，返回最终回复
    final_reply = self._generate_final_reply(user_msg, messages)
    return self._build_text_response(final_reply)

  def chat_stream(self, messages, tools, temperature=0.1, effort=None):
    """模拟流式输出：复用chat()决策逻辑，逐token yield"""
    response = self.chat(messages, tools, temperature, effort)
    msg = response["choices"][0]["message"]

    reasoning = self._generate_reasoning(messages, msg)
    if reasoning:
      for i in range(0, len(reasoning), 4):
        yield {"type": "reasoning", "text": reasoning[i:i + 4]}
        time.sleep(0.005)

    content = msg.get("content")
    if content:
      text = content
      step = 2
      for i in range(0, len(text), step):
        yield {"type": "content", "text": text[i:i + step]}
        time.sleep(0.01)

    yield {"type": "done", "message": msg}

  def _build_function_response(self, name, arguments):
    return {
        "choices": [{
            "message": {
                "role": "assistant",
                "content": None,
                "tool_calls": [{
                    "id": f"call_{name}_{id(self)}",
                    "type": "function",
                    "function": {
                        "name": name,
                        "arguments": json.dumps(arguments, ensure_ascii=False)
                    }
                }]
            },
            "finish_reason": "tool_calls"
        }]
    }

  def _build_text_response(self, content):
    return {
        "choices": [{
            "message": {
                "role": "assistant",
                "content": content
            },
            "finish_reason": "stop"
        }]
    }

  def _generate_final_reply(self, user_msg, messages):
    # 简单拼接执行结果
    results = []
    for m in messages:
      if m["role"] == "tool":
        try:
          data = json.loads(m["content"])
          if "message" in data:
            results.append(data["message"])
          elif "range_km" in data:
            results.append(f"剩余续航{data['range_km']}公里")
        except Exception:
          pass
    if results:
      return "已为你完成操作：" + "、".join(results)
    return "好的，已处理。"

  def _generate_reasoning(self, messages, msg):
    """生成模拟思考内容"""
    user_msg = ""
    for m in reversed(messages):
      if m["role"] == "user":
        user_msg = m["content"]
        break
    tool_calls = msg.get("tool_calls")
    if tool_calls:
      tool_name = tool_calls[0]["function"]["name"]
      return f"用户说「{user_msg}」，需要调用{tool_name}来处理。"
    return f"用户说「{user_msg}」，直接回复即可。"


class OpenAICompatibleLLM:
  """真实OpenAI兼容接口客户端（Ollama/vLLM/云端接口通用）"""

  def __init__(self, base_url, api_key="sk-xxx", model="deepseek-v4-flash",
               provider_id=None):
    self.configure(base_url, api_key, model, provider_id)

  def configure(self, base_url, api_key, model, provider_id=None):
    """运行期重新配置供应商参数"""
    self.api_key = api_key
    self.model = model
    self.provider_id = provider_id
    self.base_url = base_url

  def chat(self, messages, tools, temperature=0.1, effort=None):
    url = f"{self.base_url}/chat/completions"
    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {self.api_key}"
    }
    payload = {
        "model": self.model,
        "messages": messages,
        "tools": tools,
        "tool_choice": "auto",
        "temperature": temperature,
        **build_effort_payload(effort, self.provider_id)
    }
    resp = requests.post(url, headers=headers, json=payload, timeout=30)
    resp.raise_for_status()
    return resp.json()

  def chat_stream(self, messages, tools, temperature=0.1, effort=None):
    """流式调用：逐token yield reasoning/content，结束时yield完整message"""
    url = f"{self.base_url}/chat/completions"
    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {self.api_key}"
    }
    payload = {
        "model": self.model,
        "messages": messages,
        "tools": tools,
        "tool_choice": "auto",
        "temperature": temperature,
        "stream": True,
        **build_effort_payload(effort, self.provider_id)
    }

    resp = requests.post(url, headers=headers, json=payload, timeout=30, stream=True)
    try:
      resp.raise_for_status()

      content_parts = []
      reasoning_parts = []
      tool_calls_acc = {}

      for line in resp.iter_lines(decode_unicode=True):
        if not line:
          continue
        if line.startswith("data: "):  # type: ignore
          data_str = line[6:]
        elif line.startswith("data:"):  # type: ignore
          data_str = line[5:]
        else:
          continue

        data_str = data_str.strip()
        if data_str == "[DONE]":
          break

        try:
          chunk = json.loads(data_str)
        except json.JSONDecodeError:
          continue

        choices = chunk.get("choices", [])
        if not choices:
          continue
        delta = choices[0].get("delta", {})

        if delta.get("reasoning_content"):
          text = delta["reasoning_content"]
          reasoning_parts.append(text)
          yield {"type": "reasoning", "text": text}

        if delta.get("content"):
          text = delta["content"]
          content_parts.append(text)
          yield {"type": "content", "text": text}

        for tc in delta.get("tool_calls", []):
          idx = tc.get("index", 0)
          if idx not in tool_calls_acc:
            tool_calls_acc[idx] = {
                "id": "",
                "type": "function",
                "function": {"name": "", "arguments": ""}
            }
          if tc.get("id"):
            tool_calls_acc[idx]["id"] = tc["id"]
          fn = tc.get("function", {})
          if fn.get("name"):
            tool_calls_acc[idx]["function"]["name"] += fn["name"]
          if fn.get("arguments"):
            tool_calls_acc[idx]["function"]["arguments"] += fn["arguments"]

      msg = {
          "role": "assistant",
          "content": "".join(content_parts) if content_parts else None
      }
      if tool_calls_acc:
        msg["tool_calls"] = [tool_calls_acc[i] for i in sorted(tool_calls_acc)]

      yield {"type": "done", "message": msg}
    finally:
      resp.close()
