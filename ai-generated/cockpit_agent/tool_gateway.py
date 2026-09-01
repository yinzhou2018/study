from config import MOCK_VEHICLE_STATE


class ToolGateway:
  def __init__(self, toolset_manager):
    self.toolset_manager = toolset_manager
    self.vehicle_state = MOCK_VEHICLE_STATE.copy()

  def check_whitelist(self, tool_name: str) -> bool:
    """校验工具是否在当前激活工具集的白名单内"""
    current_tools = self.toolset_manager.get_current_tools()
    tool_names = [t["function"]["name"] for t in current_tools]
    return tool_name in tool_names

  def check_security(self, tool_name: str, arguments: dict) -> tuple:
    """安全规则校验，返回(是否通过, 提示信息)"""
    speed = self.vehicle_state["speed"]

    # 车窗控制：高速限制
    if tool_name == "ctrl_window_left_front":
      openness = arguments.get("openness", 0)
      if speed > 60 and openness > 25:
        return False, f"当前车速{speed}km/h，车窗开度最大25%"

    # 天窗翘角：无强限制，但高速不建议全开（此处仅翘角，放行）
    if tool_name == "ctrl_sunroof_tilt":
      pass

    return True, "安全校验通过"

  def execute(self, tool_name: str, arguments: dict) -> dict:
    """执行工具，返回结果"""
    # 先校验白名单
    if not self.check_whitelist(tool_name):
      return {"status": "failed", "message": "工具不在当前激活工具集白名单内"}

    # 再校验安全规则
    safe, msg = self.check_security(tool_name, arguments)
    if not safe:
      return {"status": "failed", "message": msg}

    # 模拟执行各工具
    if tool_name == "sys_query_vehicle_speed":
      return {"status": "success", "speed": self.vehicle_state["speed"]}

    elif tool_name == "ctrl_window_left_front":
      self.vehicle_state["window_left_front"] = arguments["openness"]
      return {"status": "success", "message": f"左前车窗已调至{arguments['openness']}%"}

    elif tool_name == "ctrl_sunroof_tilt":
      self.vehicle_state["sunroof_tilt"] = arguments["status"]
      text = "开启" if arguments["status"] == "on" else "关闭"
      return {"status": "success", "message": f"天窗翘角已{text}"}

    elif tool_name == "ctrl_ac_circulation":
      self.vehicle_state["ac_circulation"] = arguments["mode"]
      mode_map = {"inner": "内循环", "outer": "外循环", "auto": "自动"}
      return {"status": "success", "message": f"空调已切换至{mode_map[arguments['mode']]}"}

    elif tool_name == "ctrl_music_play":
      cat_map = {"light": "轻音乐", "pop": "流行音乐", "classic": "古典音乐", "radio": "电台"}
      return {"status": "success", "message": f"正在播放{cat_map[arguments['category']]}"}

    elif tool_name == "ctrl_volume_set":
      self.vehicle_state["media_volume"] = arguments["level"]
      return {"status": "success", "message": f"音量已调至{arguments['level']}"}

    elif tool_name == "ctrl_ac_temperature":
      self.vehicle_state["ac_temperature"] = arguments["temperature"]
      return {"status": "success", "message": f"空调温度已调至{arguments['temperature']}度"}

    elif tool_name == "ctrl_seat_heating":
      level = arguments["level"]
      return {"status": "success", "message": f"主驾座椅加热已调至{level}档" if level > 0 else "主驾座椅加热已关闭"}

    elif tool_name == "query_range":
      return {"status": "success", "range_km": self.vehicle_state["range"]}

    elif tool_name == "query_tire_pressure":
      return {"status": "success", "tires": {"左前": 2.4, "右前": 2.4, "左后": 2.3, "右后": 2.3}}

    elif tool_name == "list_active_toolsets":
      ids = self.toolset_manager.get_active_toolset_ids()
      return {"status": "success", "active_toolsets": ids}

    else:
      return {"status": "failed", "message": "未知工具"}
