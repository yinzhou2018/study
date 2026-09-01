import json

import requests


class MockLLMClient:
  """模拟LLM客户端，按预设逻辑返回工具调用，用于本地跑通全流程"""

  def __init__(self):
    self.step = 0  # 对话步骤计数

  def chat(self, messages, tools, temperature=0.1):
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
        return self._build_function_response("ctrl_window_left_front", {"openness": 25})

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
    if "query_range" in tool_names:
      has_range_result = any(
          m["role"] == "tool" and "range_km" in m["content"]
          for m in messages
      )
      if not has_range_result:
        return self._build_function_response("query_range", {})

    # 所有工具执行完毕，返回最终回复
    final_reply = self._generate_final_reply(user_msg, messages)
    return self._build_text_response(final_reply)

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
        except:
          pass
    if results:
      return "已为你完成操作：" + "、".join(results)
    return "好的，已处理。"


class OpenAICompatibleLLM:
  """真实OpenAI兼容接口客户端（Ollama/vLLM/云端接口通用）"""

  def __init__(self, base_url, api_key="sk-xxx", model="deepseek-v4-flash"):
    self.base_url = base_url.rstrip("/")
    self.api_key = api_key
    self.model = model

  def chat(self, messages, tools, temperature=0.1):
    url = f"{self.base_url}/v1/chat/completions"
    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {self.api_key}"
    }
    payload = {
        "model": self.model,
        "messages": messages,
        "tools": tools,
        "tool_choice": "auto",
        "temperature": temperature
    }
    resp = requests.post(url, headers=headers, json=payload, timeout=30)
    resp.raise_for_status()
    return resp.json()
