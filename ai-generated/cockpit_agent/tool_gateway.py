from config import AUDIO_ZONES, DEFAULT_ZONE, MOCK_VEHICLE_STATE


# 车窗位置 → (状态键, 中文标签)
WINDOW_POSITION_MAP = {
    "front_left": ("window_front_left", "左前车窗"),
    "front_right": ("window_front_right", "右前车窗"),
    "rear_left": ("window_rear_left", "左后车窗"),
    "rear_right": ("window_rear_right", "右后车窗"),
}


# 座椅位置 → 中文标签
SEAT_POSITION_LABELS = {
    "front_left": "主驾",
    "front_right": "副驾",
    "rear": "后排",
}

# 雨刮位置 → 中文标签
WIPER_POSITION_LABELS = {
    "front": "前",
    "rear": "后",
}


# 这些能力涉及行车安全、车辆门锁或对外放电，不允许非主驾音区直接触发。
DRIVER_ONLY_TOOLS = {
    "ctrl_door_lock",
    "ctrl_child_lock",
    "ctrl_tailgate",
    "ctrl_door_soft_close",
    "ctrl_cruise_control",
    "ctrl_lane_keep",
    "ctrl_auto_park",
    "ctrl_remote_park",
    "ctrl_charge_start",
    "ctrl_charge_stop",
    "ctrl_v2l_discharge",
}


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

    # 车窗控制：高速限制（对所有位置生效）
    if tool_name == "ctrl_window":
      openness = arguments.get("openness", 50)
      if speed > 60 and openness > 25:
        return False, f"当前车速{speed}km/h，车窗开度最大25%"

    # 天窗翘角：无强限制，但高速不建议全开（此处仅翘角，放行）
    if tool_name == "ctrl_sunroof_tilt":
      pass

    return True, "安全校验通过"

  def check_permission(self, tool_name: str, requesting_zone: str) -> tuple:
    """校验发起音区是否有权限调用该工具"""
    if requesting_zone not in AUDIO_ZONES:
      return False, f"未知音区: {requesting_zone}"
    if requesting_zone != "front_left" and tool_name in DRIVER_ONLY_TOOLS:
      return False, f"{requesting_zone}音区无权限调用{tool_name}"
    return True, "权限校验通过"

  def execute(self, tool_name: str, arguments: dict,
              requesting_zone: str = DEFAULT_ZONE) -> dict:
    """执行工具，返回结果"""
    # 先校验白名单
    if not self.check_whitelist(tool_name):
      return {"status": "failed", "message": "工具不在当前激活工具集白名单内"}

    # 再校验音区权限
    allowed, permission_msg = self.check_permission(tool_name, requesting_zone)
    if not allowed:
      return {"status": "failed", "message": permission_msg}

    # 再校验安全规则
    safe, msg = self.check_security(tool_name, arguments)
    if not safe:
      return {"status": "failed", "message": msg}

    # 模拟执行各工具
    if tool_name == "sys_query_vehicle_speed":
      return {"status": "success", "speed": self.vehicle_state["speed"]}

    elif tool_name == "ctrl_window":
      return self._execute_window(arguments)

    elif tool_name == "ctrl_sunroof_tilt":
      self.vehicle_state["sunroof_tilt"] = arguments["status"]
      text = "开启" if arguments["status"] == "on" else "关闭"
      return {"status": "success", "message": f"天窗翘角已{text}"}

    elif tool_name == "ctrl_ac_power":
      self.vehicle_state["ac_power"] = arguments["status"]
      text = "开启" if arguments["status"] == "on" else "关闭"
      return {"status": "success", "message": f"空调已{text}"}

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

    elif tool_name == "ctrl_seat_heat":
      return self._execute_seat_heat(arguments)

    elif tool_name == "ctrl_seat_vent":
      return self._execute_seat_vent(arguments)

    elif tool_name == "ctrl_seat_massage":
      return self._execute_seat_massage(arguments)

    elif tool_name == "ctrl_wiper":
      return self._execute_wiper(arguments)

    elif tool_name == "query_range":
      return {"status": "success", "range_km": self.vehicle_state["range"]}

    elif tool_name == "query_tire_pressure":
      return {"status": "success", "tires": {"左前": 2.4, "右前": 2.4, "左后": 2.3, "右后": 2.3}}

    elif tool_name == "list_active_toolsets":
      ids = self.toolset_manager.get_active_toolset_ids()
      return {"status": "success", "active_toolsets": ids}

    else:
      return {"status": "failed", "message": "未知工具"}

  def _execute_window(self, arguments):
    """车窗控制：解析位置与开度，落库状态并返回中文提示"""
    position = arguments["position"]
    openness = arguments.get("openness", 50)
    self._set_window_state(position, openness)
    return {"status": "success", "message": f"{self._window_label(position)}已调至{openness}%"}

  def _set_window_state(self, position, openness):
    """按位置写入对应车窗状态键，all 联动全车"""
    keys = [v[0] for v in WINDOW_POSITION_MAP.values()] if position == "all" \
        else [WINDOW_POSITION_MAP[position][0]]
    for key in keys:
      self.vehicle_state[key] = openness

  def _window_label(self, position):
    """返回位置的中文播报标签"""
    if position == "all":
      return "全车车窗"
    return WINDOW_POSITION_MAP[position][1]

  def _execute_seat_heat(self, arguments):
    """座椅加热：按位置落库档位并回填文案"""
    position = arguments["position"]
    level = arguments["level"]
    self.vehicle_state[f"seat_heat_{position}"] = level
    label = SEAT_POSITION_LABELS[position]
    if level == 0:
      return {"status": "success", "message": f"{label}座椅加热已关闭"}
    return {"status": "success", "message": f"{label}座椅加热已调至{level}档"}

  def _execute_seat_vent(self, arguments):
    """座椅通风：按位置落库档位并回填文案"""
    position = arguments["position"]
    level = arguments["level"]
    self.vehicle_state[f"seat_vent_{position}"] = level
    label = SEAT_POSITION_LABELS[position]
    if level == 0:
      return {"status": "success", "message": f"{label}座椅通风已关闭"}
    return {"status": "success", "message": f"{label}座椅通风已调至{level}档"}

  def _execute_seat_massage(self, arguments):
    """座椅按摩：按位置落库模式并回填文案"""
    position = arguments["position"]
    mode = arguments["mode"]
    level = arguments["level"]
    self.vehicle_state[f"seat_massage_{position}"] = mode
    label = SEAT_POSITION_LABELS[position]
    if mode == "off":
      return {"status": "success", "message": f"{label}座椅按摩已关闭"}
    return {"status": "success", "message": f"{label}座椅按摩{mode}模式{level}档"}

  def _execute_wiper(self, arguments):
    """雨刮：按位置落库档位并回填文案"""
    position = arguments["position"]
    level = arguments["level"]
    self.vehicle_state[f"wiper_{position}"] = level
    label = WIPER_POSITION_LABELS[position]
    if level == "off":
      return {"status": "success", "message": f"{label}雨刮已关闭"}
    return {"status": "success", "message": f"{label}雨刮已调至{level}档"}
