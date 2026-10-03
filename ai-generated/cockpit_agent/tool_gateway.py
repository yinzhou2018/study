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


# ===== 辅助函数 =====

def _window_label(position):
  """返回车窗位置的中文播报标签"""
  if position == "all":
    return "全车车窗"
  return WINDOW_POSITION_MAP[position][1]


def _set_window_state(state, position, openness):
  """按位置写入对应车窗状态键，all 联动全车"""
  keys = [v[0] for v in WINDOW_POSITION_MAP.values()] if position == "all" \
      else [WINDOW_POSITION_MAP[position][0]]
  for key in keys:
    state[key] = openness


def _ok(msg):
  """构造成功响应"""
  return {"status": "success", "message": msg}


# ===== 车身控制工具集 =====

def _h_window(state, args):
  position = args["position"]
  openness = args.get("openness", 50)
  _set_window_state(state, position, openness)
  return _ok(f"{_window_label(position)}已调至{openness}%")


def _h_sunroof_tilt(state, args):
  state["sunroof_tilt"] = args["status"]
  return _ok(f"天窗翘角已{'开启' if args['status'] == 'on' else '关闭'}")


def _h_sunroof_open(state, args):
  openness = args.get("openness", 50)
  state["sunroof_open"] = openness
  return _ok(f"天窗已开启{openness}%")


def _h_sunroof_shade(state, args):
  openness = args.get("openness", 50)
  state["sunroof_shade"] = openness
  return _ok(f"天窗遮阳帘已调至{openness}%")


def _h_door_lock(state, args):
  state["door_lock"] = args["status"]
  return _ok(f"全车门锁已{'上锁' if args['status'] == 'lock' else '解锁'}")


def _h_child_lock(state, args):
  state["child_lock"] = args["status"]
  return _ok(f"儿童锁已{'开启' if args['status'] == 'on' else '关闭'}")


def _h_tailgate(state, args):
  state["tailgate"] = args["action"]
  return _ok(f"尾门已{'打开' if args['action'] == 'open' else '关闭'}")


def _h_door_soft_close(state, args):
  door = args["position"]
  label = {"front_left": "左前门", "front_right": "右前门", "tailgate": "尾门"}[door]
  return _ok(f"{label}已辅助吸合关闭")


def _h_door_handle(state, args):
  door = args["position"]
  state[f"door_handle_{door}"] = args["status"]
  label = {"front_left": "左前", "front_right": "右前"}[door]
  return _ok(f"{label}门把手已{'弹出' if args['status'] == 'pop' else '收回'}")


def _h_mirror_fold(state, args):
  state["mirror_fold"] = args["status"]
  return _ok(f"外后视镜已{'折叠' if args['status'] == 'fold' else '展开'}")


def _h_wiper(state, args):
  position = args["position"]
  level = args["level"]
  state[f"wiper_{position}"] = level
  label = WIPER_POSITION_LABELS[position]
  if level == "off":
    return _ok(f"{label}雨刮已关闭")
  return _ok(f"{label}雨刮已调至{level}档")


def _q_window_status(state, args):
  return {"status": "success",
          "front_left": state["window_front_left"],
          "front_right": state["window_front_right"],
          "rear_left": state["window_rear_left"],
          "rear_right": state["window_rear_right"]}


def _q_sunroof_tilt(state, args):
  return {"status": "success", "tilt": state["sunroof_tilt"]}


def _q_sunroof_open(state, args):
  return {"status": "success", "openness": state["sunroof_open"]}


def _q_sunroof_shade(state, args):
  return {"status": "success", "openness": state["sunroof_shade"]}


def _q_door_lock_status(state, args):
  return {"status": "success", "door_lock": state["door_lock"]}


def _q_child_lock_status(state, args):
  return {"status": "success", "child_lock": state["child_lock"]}


def _q_tailgate_status(state, args):
  return {"status": "success", "tailgate": state["tailgate"]}


def _q_door_handle_status(state, args):
  return {"status": "success",
          "front_left": state["door_handle_front_left"],
          "front_right": state["door_handle_front_right"]}


def _q_mirror_fold_status(state, args):
  return {"status": "success", "mirror_fold": state["mirror_fold"]}


def _q_wiper_status(state, args):
  return {"status": "success", "front": state["wiper_front"],
          "rear": state["wiper_rear"]}


# ===== 灯光照明工具集 =====

def _h_headlight_mode(state, args):
  state["headlight_mode"] = args["mode"]
  return _ok(f"大灯已切换至{args['mode']}模式")


def _h_drl(state, args):
  state["drl"] = args["status"]
  return _ok(f"日行灯已{'开启' if args['status'] == 'on' else '关闭'}")


def _h_fog_light(state, args):
  state["fog_light"] = args["status"]
  return _ok(f"雾灯已{'开启' if args['status'] == 'on' else '关闭'}")


def _h_ambient_light(state, args):
  state["ambient_light_color"] = args["color"]
  state["ambient_light_brightness"] = args["brightness"]
  return _ok(f"氛围灯已设为{args['color']}色，亮度{args['brightness']}%")


def _h_ambient_scene(state, args):
  state["ambient_scene"] = args["scene"]
  return _ok(f"氛围灯已切换至{args['scene']}场景")


def _h_welcome_light(state, args):
  state["welcome_light"] = args["status"]
  return _ok(f"迎宾灯已{'开启' if args['status'] == 'on' else '关闭'}")


def _h_star_roof(state, args):
  state["star_roof_brightness"] = args["brightness"]
  state["star_roof_mode"] = args["mode"]
  return _ok(f"星空顶已设为{args['mode']}模式，亮度{args['brightness']}%")


def _h_interior_reading(state, args):
  position = args["position"]
  status = args["status"]
  keys = ["interior_reading_front_left", "interior_reading_front_right",
          "interior_reading_rear"] if position == "all" \
      else [f"interior_reading_{position}"]
  for key in keys:
    state[key] = status
  label = {"front_left": "主驾", "front_right": "副驾",
           "rear": "后排", "all": "全车"}[position]
  return _ok(f"{label}阅读灯已{status}")


def _q_headlight_mode(state, args):
  return {"status": "success", "headlight_mode": state["headlight_mode"]}


def _q_drl_status(state, args):
  return {"status": "success", "drl": state["drl"]}


def _q_fog_light_status(state, args):
  return {"status": "success", "fog_light": state["fog_light"]}


def _q_ambient_light_status(state, args):
  return {"status": "success", "color": state["ambient_light_color"],
          "brightness": state["ambient_light_brightness"]}


def _q_ambient_scene(state, args):
  return {"status": "success", "scene": state["ambient_scene"]}


def _q_welcome_light_status(state, args):
  return {"status": "success", "welcome_light": state["welcome_light"]}


def _q_star_roof_status(state, args):
  return {"status": "success", "brightness": state["star_roof_brightness"],
          "mode": state["star_roof_mode"]}


def _q_reading_light_status(state, args):
  return {"status": "success",
          "front_left": state["interior_reading_front_left"],
          "front_right": state["interior_reading_front_right"],
          "rear": state["interior_reading_rear"]}


# ===== 座椅系统工具集 =====

def _h_seat_memory(state, args):
  state["seat_memory"] = args["slot"]
  return _ok(f"主驾座椅已调用记忆{args['slot']}档")


def _h_seat_heat(state, args):
  position = args["position"]
  level = args["level"]
  state[f"seat_heat_{position}"] = level
  label = SEAT_POSITION_LABELS[position]
  if level == 0:
    return _ok(f"{label}座椅加热已关闭")
  return _ok(f"{label}座椅加热已调至{level}档")


def _h_seat_vent(state, args):
  position = args["position"]
  level = args["level"]
  state[f"seat_vent_{position}"] = level
  label = SEAT_POSITION_LABELS[position]
  if level == 0:
    return _ok(f"{label}座椅通风已关闭")
  return _ok(f"{label}座椅通风已调至{level}档")


def _h_seat_massage(state, args):
  position = args["position"]
  mode = args["mode"]
  level = args["level"]
  state[f"seat_massage_{position}"] = mode
  state[f"seat_massage_level_{position}"] = level
  label = SEAT_POSITION_LABELS[position]
  if mode == "off":
    return _ok(f"{label}座椅按摩已关闭")
  return _ok(f"{label}座椅按摩{mode}模式{level}档")


def _h_seat_lumbar(state, args):
  state["seat_lumbar"] = args["level"]
  return _ok(f"主驾腰托已调至{args['level']}档")


def _h_boss_key(state, args):
  state["seat_boss_key"] = args["status"]
  return _ok(f"老板键已{'开启' if args['status'] == 'on' else '关闭'}")


def _h_seat_recline(state, args):
  state["seat_recline"] = args["angle"]
  return _ok(f"后排座椅靠背已调至{args['angle']}档")


def _q_seat_heat_status(state, args):
  return {"status": "success",
          "front_left": state["seat_heat_front_left"],
          "front_right": state["seat_heat_front_right"],
          "rear": state["seat_heat_rear"]}


def _q_seat_vent_status(state, args):
  return {"status": "success",
          "front_left": state["seat_vent_front_left"],
          "front_right": state["seat_vent_front_right"],
          "rear": state["seat_vent_rear"]}


def _q_seat_massage_status(state, args):
  return {"status": "success",
          "front_left": {"mode": state["seat_massage_front_left"],
                         "level": state.get("seat_massage_level_front_left")},
          "front_right": {"mode": state["seat_massage_front_right"],
                          "level": state.get("seat_massage_level_front_right")}}


def _q_seat_lumbar_status(state, args):
  return {"status": "success", "level": state["seat_lumbar"]}


def _q_seat_boss_key_status(state, args):
  return {"status": "success", "boss_key": state["seat_boss_key"]}


def _q_seat_recline_status(state, args):
  return {"status": "success", "angle": state["seat_recline"]}


# ===== 空调温控工具集 =====

def _h_ac_power(state, args):
  state["ac_power"] = args["status"]
  return _ok(f"空调已{'开启' if args['status'] == 'on' else '关闭'}")


def _h_ac_temperature(state, args):
  state["ac_temperature"] = args["temperature"]
  return _ok(f"空调温度已调至{args['temperature']}度")


def _h_ac_fan_speed(state, args):
  state["ac_fan_speed"] = args["level"]
  return _ok(f"空调风量已调至{args['level']}档")


def _h_ac_circulation(state, args):
  state["ac_circulation"] = args["mode"]
  mode_map = {"inner": "内循环", "outer": "外循环", "auto": "自动"}
  return _ok(f"空调已切换至{mode_map[args['mode']]}")


def _h_ac_vent_mode(state, args):
  state["ac_vent_mode"] = args["mode"]
  return _ok(f"出风模式已切换为{args['mode']}")


def _h_air_purifier(state, args):
  state["air_purifier"] = args["status"]
  return _ok(f"空气净化器已{'开启' if args['status'] == 'on' else '关闭'}")


def _h_ionizer(state, args):
  state["ionizer"] = args["status"]
  return _ok(f"负离子发生器已{'开启' if args['status'] == 'on' else '关闭'}")


def _q_air_quality(state, args):
  return {"status": "success", "pm25": 12, "co2": 450,
          "purifier": state["air_purifier"], "ionizer": state["ionizer"]}


def _q_ac_power_status(state, args):
  return {"status": "success", "ac_power": state["ac_power"]}


def _q_ac_temperature(state, args):
  return {"status": "success", "temperature": state["ac_temperature"]}


def _q_ac_fan_speed(state, args):
  return {"status": "success", "fan_speed": state["ac_fan_speed"]}


def _q_ac_circulation(state, args):
  return {"status": "success", "circulation": state["ac_circulation"]}


def _q_ac_vent_mode(state, args):
  return {"status": "success", "vent_mode": state["ac_vent_mode"]}


def _q_air_purifier_status(state, args):
  return {"status": "success", "air_purifier": state["air_purifier"]}


def _q_ionizer_status(state, args):
  return {"status": "success", "ionizer": state["ionizer"]}


# ===== 座舱舒适工具集 =====

def _h_fridge(state, args):
  state["fridge_mode"] = args["mode"]
  mode_map = {"off": "关闭", "cold": "制冷", "warm": "制热"}
  return _ok(f"车载冰箱已{mode_map[args['mode']]}")


def _h_fridge_temp(state, args):
  state["fridge_temp"] = args["temperature"]
  return _ok(f"冰箱温度已设为{args['temperature']}度")


def _h_aroma(state, args):
  state["aroma_scent"] = args["scent"]
  state["aroma_intensity"] = args["intensity"]
  return _ok(f"香氛已设为{args['scent']}香型，{args['intensity']}浓度")


def _h_panoramic_shade(state, args):
  openness = args.get("openness", 50)
  state["panoramic_shade"] = openness
  return _ok(f"全景天幕遮阳帘已调至{openness}%")


def _h_armrest_heat(state, args):
  state["armrest_heat"] = args["status"]
  return _ok(f"中央扶手箱加热已{'开启' if args['status'] == 'on' else '关闭'}")


def _q_fridge_status(state, args):
  return {"status": "success", "mode": state["fridge_mode"]}


def _q_fridge_temp(state, args):
  return {"status": "success", "temperature": state["fridge_temp"]}


def _q_aroma_status(state, args):
  return {"status": "success", "scent": state["aroma_scent"],
          "intensity": state["aroma_intensity"]}


def _q_panoramic_shade_status(state, args):
  return {"status": "success", "openness": state["panoramic_shade"]}


def _q_armrest_heat_status(state, args):
  return {"status": "success", "armrest_heat": state["armrest_heat"]}


# ===== 影音娱乐工具集 =====

def _h_music_play(state, args):
  state["music_category"] = args["category"]
  state["music_track"] = 1
  cat_map = {"light": "轻音乐", "pop": "流行音乐", "classic": "古典音乐",
             "radio": "电台", "favorite": "收藏歌单"}
  return _ok(f"正在播放{cat_map[args['category']]}")


def _h_music_next(state, args):
  state["music_track"] = state.get("music_track", 0) + 1
  return _ok("已切换至下一首")


def _h_music_prev(state, args):
  state["music_track"] = max(1, state.get("music_track", 1) - 1)
  return _ok("已切换至上一首")


def _h_volume_media(state, args):
  state["volume_media"] = args["level"]
  return _ok(f"媒体音量已调至{args['level']}")


def _h_sound_mode(state, args):
  state["sound_mode"] = args["mode"]
  return _ok(f"音效已切换为{args['mode']}模式")


def _h_radio_tune(state, args):
  state["radio_frequency"] = args["frequency"]
  state["music_category"] = "radio"
  return _ok(f"已切换至电台{args['frequency']}")


def _h_media_mute(state, args):
  state["media_mute"] = args["status"]
  return _ok(f"媒体已{'静音' if args['status'] == 'on' else '取消静音'}")


def _h_rear_entertainment(state, args):
  state["rear_entertainment"] = args["status"]
  return _ok(f"后排娱乐系统已{'开启' if args['status'] == 'on' else '关闭'}")


def _q_music_status(state, args):
  return {"status": "success", "category": state["music_category"],
          "track": state["music_track"]}


def _q_volume_media(state, args):
  return {"status": "success", "level": state["volume_media"],
          "mute": state["media_mute"]}


def _q_sound_mode_status(state, args):
  return {"status": "success", "mode": state["sound_mode"]}


def _q_radio_status(state, args):
  return {"status": "success", "frequency": state["radio_frequency"]}


def _q_rear_entertainment_status(state, args):
  return {"status": "success", "enabled": state["rear_entertainment"]}


# ===== 导航出行工具集 =====

def _h_nav_start(state, args):
  state["nav_active"] = True
  state["nav_destination"] = args["destination"]
  return _ok(f"已发起导航至{args['destination']}")


def _h_nav_stop(state, args):
  state["nav_active"] = False
  state["nav_destination"] = None
  return _ok("已结束导航")


def _h_nav_route_pref(state, args):
  state["nav_route_pref"] = args["mode"]
  pref_map = {"fastest": "最快", "shortest": "最短",
              "no_highway": "避高速", "no_toll": "避收费"}
  return _ok(f"路线偏好已设为{pref_map[args['mode']]}")


def _q_nav_status(state, args):
  return {"status": "success", "active": state["nav_active"],
          "destination": state["nav_destination"],
          "route_pref": state["nav_route_pref"]}


def _q_poi_nearby(state, args):
  return {"status": "success", "type": args["type"],
          "results": [f"附近{args['keyword']}A店", f"附近{args['keyword']}B店"]}


def _q_traffic_status(state, args):
  return {"status": "success", "current_road": "主干道",
          "congestion": "畅通", "delay_min": 0}


def _q_charging_station(state, args):
  return {"status": "success", "nearby": [
      {"name": "超充站A", "distance_km": 2.5, "available": 4},
      {"name": "快充站B", "distance_km": 5.0, "available": 2}]}


def _q_route_charge_plan(state, args):
  return {"status": "success", "soc_threshold": args["soc_threshold"],
          "planned_stops": 1, "next_charge_km": 180}


# ===== 车辆信息工具集 =====

def _q_battery_status(state, args):
  return {"status": "success", "soc": state["battery_soc"],
          "charging": state["charge_active"], "limit": state["charge_limit"]}


def _q_range_mileage(state, args):
  return {"status": "success", "range_km": state["range"],
          "soc": state["battery_soc"]}


def _q_energy_consumption(state, args):
  return {"status": "success", "avg_kwh_per_100km": 15.6,
          "today_kwh": 4.2, "regen_kwh": 0.8}


def _q_tire_pressure(state, args):
  return {"status": "success", "tires": {
      "左前": 2.4, "右前": 2.4, "左后": 2.3, "右后": 2.3}}


# ===== 充电管理工具集 =====

def _h_charge_start(state, args):
  state["charge_active"] = True
  return _ok("已开始充电")


def _h_charge_stop(state, args):
  state["charge_active"] = False
  return _ok("已停止充电")


def _h_charge_limit(state, args):
  state["charge_limit"] = args["percent"]
  return _ok(f"充电上限已设为{args['percent']}%")


def _h_charge_schedule(state, args):
  state["charge_schedule_start"] = args["start_time"]
  state["charge_schedule_end"] = args["end_time"]
  return _ok(f"已预约充电{args['start_time']}-{args['end_time']}")


def _h_v2l_discharge(state, args):
  state["v2l_discharge"] = args["status"]
  return _ok(f"对外放电已{'开启' if args['status'] == 'on' else '关闭'}")


def _q_charge_status(state, args):
  return {"status": "success", "charging": state["charge_active"],
          "limit": state["charge_limit"],
          "schedule": {"start": state["charge_schedule_start"],
                       "end": state["charge_schedule_end"]},
          "v2l": state["v2l_discharge"]}


# ===== 通讯通话工具集 =====

def _h_phone_call(state, args):
  state["phone_call_state"] = args["contact"]
  return _ok(f"正在呼叫{args['contact']}")


def _h_phone_answer(state, args):
  state["phone_call_state"] = "通话中"
  return _ok("已接听来电")


def _h_phone_hangup(state, args):
  state["phone_call_state"] = None
  return _ok("已挂断通话")


def _q_contact_search(state, args):
  return {"status": "success", "keyword": args["keyword"],
          "matches": [{"name": f"{args['keyword']}·联系人1", "phone": "138****0001"}]}


def _h_message_read(state, args):
  unread = state.get("message_unread", 0)
  state["message_unread"] = 0
  return _ok(f"已播报{unread}条未读消息")


# ===== 驾驶辅助工具集 =====

def _h_cruise_control(state, args):
  state["cruise_control"] = args["status"]
  if "speed" in args:
    state["cruise_speed"] = args["speed"]
  return _ok(f"定速巡航已{'开启' if args['status'] == 'on' else '关闭'}")


def _h_lane_keep(state, args):
  state["lane_keep"] = args["status"]
  return _ok(f"车道保持已{'开启' if args['status'] == 'on' else '关闭'}")


def _h_acc_distance(state, args):
  state["acc_distance"] = args["level"]
  return _ok(f"跟车距离已调至{args['level']}档")


def _h_energy_recovery(state, args):
  state["energy_recovery"] = args["level"]
  return _ok(f"能量回收已设为{args['level']}模式")


def _h_auto_park(state, args):
  state["auto_park"] = args["mode"]
  return _ok(f"自动泊车已启动（{args['mode']}位）")


def _h_remote_park(state, args):
  state["remote_park"] = args["direction"]
  return _ok(f"遥控泊车已启动（{args['direction']}）")


def _h_hud_display(state, args):
  state["hud_display"] = args["status"]
  return _ok(f"HUD抬头显示已{'开启' if args['status'] == 'on' else '关闭'}")


def _q_cruise_control_status(state, args):
  return {"status": "success", "enabled": state["cruise_control"],
          "speed": state["cruise_speed"]}


def _q_lane_keep_status(state, args):
  return {"status": "success", "lane_keep": state["lane_keep"]}


def _q_acc_distance_status(state, args):
  return {"status": "success", "level": state["acc_distance"]}


def _q_energy_recovery_status(state, args):
  return {"status": "success", "mode": state["energy_recovery"]}


def _q_hud_status(state, args):
  return {"status": "success", "hud_display": state["hud_display"]}


# ===== 座舱模式工具集（内部联动若干 ctrl） =====

def _trigger(name, state, args):
  """内部联动调用，绕过校验直接执行 handler"""
  return TOOL_HANDLERS[name](state, args)


def _h_car_wash_mode(state, args):
  _trigger("ctrl_window", state, {"position": "all", "openness": 0})
  _trigger("ctrl_sunroof_open", state, {"openness": 0})
  _trigger("ctrl_mirror_fold", state, {"status": "fold"})
  return _ok("洗车模式已启动：已关闭全车车窗与天窗，折叠后视镜")


def _h_rest_mode(state, args):
  _trigger("ctrl_window", state, {"position": "all", "openness": 0})
  _trigger("ctrl_sunroof_tilt", state, {"status": "on"})
  _trigger("ctrl_ac_temperature", state, {"temperature": 24})
  _trigger("ctrl_ambient_scene", state, {"scene": "rest"})
  return _ok("休息模式已启动：已关窗、天窗翘角通风、空调24度、氛围灯休息场景")


def _h_commute_mode(state, args):
  _trigger("ctrl_music_play", state, {"category": "light"})
  _trigger("ctrl_volume_media", state, {"level": 15})
  return _ok("通勤模式已启动：正在播放轻音乐，音量15")


def _h_child_mode(state, args):
  _trigger("ctrl_child_lock", state, {"status": "on"})
  _trigger("ctrl_window", state, {"position": "rear_left", "openness": 0})
  _trigger("ctrl_window", state, {"position": "rear_right", "openness": 0})
  return _ok("儿童模式已启动：已开启儿童锁、关闭后排车窗")


def _h_charge_mode(state, args):
  _trigger("ctrl_charge_limit", state, {"percent": 80})
  _trigger("ctrl_ac_power", state, {"status": "off"})
  return _ok("充电节能模式已启动：充电上限80%，空调关闭")


def _h_camp_mode(state, args):
  _trigger("ctrl_v2l_discharge", state, {"status": "on"})
  _trigger("ctrl_window", state, {"position": "all", "openness": 0})
  _trigger("ctrl_ambient_scene", state, {"scene": "rest"})
  return _ok("露营模式已启动：已开启对外放电、关闭车窗、氛围灯休息场景")


def _h_pet_mode(state, args):
  _trigger("ctrl_ac_power", state, {"status": "on"})
  _trigger("ctrl_ac_temperature", state, {"temperature": 22})
  _trigger("ctrl_window", state, {"position": "all", "openness": 10})
  return _ok("宠物模式已启动：空调22度、车窗留缝10%")


def _h_stealth_mode(state, args):
  _trigger("ctrl_media_mute", state, {"status": "on"})
  _trigger("ctrl_interior_reading", state, {"position": "all", "status": "off"})
  return _ok("隐私模式已启动：媒体静音、阅读灯关闭")


# ===== 工具分发表 =====

TOOL_HANDLERS = {
    # 车身控制
    "ctrl_window": _h_window, "ctrl_sunroof_tilt": _h_sunroof_tilt,
    "ctrl_sunroof_open": _h_sunroof_open, "ctrl_sunroof_shade": _h_sunroof_shade,
    "ctrl_door_lock": _h_door_lock, "ctrl_child_lock": _h_child_lock,
    "ctrl_tailgate": _h_tailgate, "ctrl_door_soft_close": _h_door_soft_close,
    "ctrl_door_handle": _h_door_handle, "ctrl_mirror_fold": _h_mirror_fold,
    "ctrl_wiper": _h_wiper,
    "query_window_status": _q_window_status,
    "query_sunroof_tilt": _q_sunroof_tilt, "query_sunroof_open": _q_sunroof_open,
    "query_sunroof_shade": _q_sunroof_shade,
    "query_door_lock_status": _q_door_lock_status,
    "query_child_lock_status": _q_child_lock_status,
    "query_tailgate_status": _q_tailgate_status,
    "query_door_handle_status": _q_door_handle_status,
    "query_mirror_fold_status": _q_mirror_fold_status,
    "query_wiper_status": _q_wiper_status,
    # 灯光照明
    "ctrl_headlight_mode": _h_headlight_mode, "ctrl_drl": _h_drl,
    "ctrl_fog_light": _h_fog_light, "ctrl_ambient_light": _h_ambient_light,
    "ctrl_ambient_scene": _h_ambient_scene, "ctrl_welcome_light": _h_welcome_light,
    "ctrl_star_roof": _h_star_roof, "ctrl_interior_reading": _h_interior_reading,
    "query_headlight_mode": _q_headlight_mode, "query_drl_status": _q_drl_status,
    "query_fog_light_status": _q_fog_light_status,
    "query_ambient_light_status": _q_ambient_light_status,
    "query_ambient_scene": _q_ambient_scene,
    "query_welcome_light_status": _q_welcome_light_status,
    "query_star_roof_status": _q_star_roof_status,
    "query_reading_light_status": _q_reading_light_status,
    # 座椅系统
    "ctrl_seat_driver_memory": _h_seat_memory, "ctrl_seat_heat": _h_seat_heat,
    "ctrl_seat_vent": _h_seat_vent, "ctrl_seat_massage": _h_seat_massage,
    "ctrl_seat_driver_lumbar": _h_seat_lumbar, "ctrl_seat_boss_key": _h_boss_key,
    "ctrl_seat_rear_recline": _h_seat_recline,
    "query_seat_heat_status": _q_seat_heat_status,
    "query_seat_vent_status": _q_seat_vent_status,
    "query_seat_massage_status": _q_seat_massage_status,
    "query_seat_lumbar_status": _q_seat_lumbar_status,
    "query_seat_boss_key_status": _q_seat_boss_key_status,
    "query_seat_recline_status": _q_seat_recline_status,
    # 空调温控
    "ctrl_ac_power": _h_ac_power, "ctrl_ac_temperature": _h_ac_temperature,
    "ctrl_ac_fan_speed": _h_ac_fan_speed, "ctrl_ac_circulation": _h_ac_circulation,
    "ctrl_ac_vent_mode": _h_ac_vent_mode, "ctrl_air_purifier": _h_air_purifier,
    "ctrl_ionizer": _h_ionizer, "query_air_quality": _q_air_quality,
    "query_ac_power_status": _q_ac_power_status,
    "query_ac_temperature": _q_ac_temperature, "query_ac_fan_speed": _q_ac_fan_speed,
    "query_ac_circulation": _q_ac_circulation, "query_ac_vent_mode": _q_ac_vent_mode,
    "query_air_purifier_status": _q_air_purifier_status,
    "query_ionizer_status": _q_ionizer_status,
    # 座舱舒适
    "ctrl_fridge": _h_fridge, "ctrl_fridge_temp": _h_fridge_temp,
    "ctrl_aroma_system": _h_aroma, "ctrl_panoramic_shade": _h_panoramic_shade,
    "ctrl_armrest_heat": _h_armrest_heat,
    "query_fridge_status": _q_fridge_status, "query_fridge_temp": _q_fridge_temp,
    "query_aroma_status": _q_aroma_status,
    "query_panoramic_shade_status": _q_panoramic_shade_status,
    "query_armrest_heat_status": _q_armrest_heat_status,
    # 影音娱乐
    "ctrl_music_play": _h_music_play, "ctrl_music_next": _h_music_next,
    "ctrl_music_prev": _h_music_prev, "ctrl_volume_media": _h_volume_media,
    "ctrl_sound_mode": _h_sound_mode, "ctrl_radio_tune": _h_radio_tune,
    "ctrl_media_mute": _h_media_mute, "ctrl_rear_entertainment": _h_rear_entertainment,
    "query_music_status": _q_music_status, "query_volume_media": _q_volume_media,
    "query_sound_mode_status": _q_sound_mode_status,
    "query_radio_status": _q_radio_status,
    "query_rear_entertainment_status": _q_rear_entertainment_status,
    # 导航出行
    "ctrl_nav_start": _h_nav_start, "ctrl_nav_stop": _h_nav_stop,
    "ctrl_nav_route_pref": _h_nav_route_pref, "query_poi_nearby": _q_poi_nearby,
    "query_traffic_status": _q_traffic_status,
    "query_charging_station": _q_charging_station,
    "query_route_charge_plan": _q_route_charge_plan,
    "query_nav_status": _q_nav_status,
    # 车辆信息
    "query_battery_status": _q_battery_status,
    "query_range_mileage": _q_range_mileage,
    "query_energy_consumption": _q_energy_consumption,
    "query_tire_pressure": _q_tire_pressure,
    # 充电管理
    "ctrl_charge_start": _h_charge_start, "ctrl_charge_stop": _h_charge_stop,
    "ctrl_charge_limit": _h_charge_limit, "ctrl_charge_schedule": _h_charge_schedule,
    "ctrl_v2l_discharge": _h_v2l_discharge, "query_charge_status": _q_charge_status,
    # 通讯通话
    "ctrl_phone_call": _h_phone_call, "ctrl_phone_answer": _h_phone_answer,
    "ctrl_phone_hangup": _h_phone_hangup, "query_contact_search": _q_contact_search,
    "ctrl_message_read": _h_message_read,
    # 驾驶辅助
    "ctrl_cruise_control": _h_cruise_control, "ctrl_lane_keep": _h_lane_keep,
    "ctrl_acc_distance": _h_acc_distance, "ctrl_energy_recovery": _h_energy_recovery,
    "ctrl_auto_park": _h_auto_park, "ctrl_remote_park": _h_remote_park,
    "ctrl_hud_display": _h_hud_display,
    "query_cruise_control_status": _q_cruise_control_status,
    "query_lane_keep_status": _q_lane_keep_status,
    "query_acc_distance_status": _q_acc_distance_status,
    "query_energy_recovery_status": _q_energy_recovery_status,
    "query_hud_status": _q_hud_status,
    # 座舱模式
    "trigger_car_wash_mode": _h_car_wash_mode, "trigger_rest_mode": _h_rest_mode,
    "trigger_commute_mode": _h_commute_mode, "trigger_child_mode": _h_child_mode,
    "trigger_charge_mode": _h_charge_mode, "trigger_camp_mode": _h_camp_mode,
    "trigger_pet_mode": _h_pet_mode, "trigger_stealth_mode": _h_stealth_mode,
    # 系统工具
    "list_active_toolsets": lambda state, args: None,  # 由 execute 特殊处理
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
    """执行工具：白名单→权限→安全→分发表调用"""
    if not self.check_whitelist(tool_name):
      return {"status": "failed", "message": "工具不在当前激活工具集白名单内"}

    allowed, permission_msg = self.check_permission(tool_name, requesting_zone)
    if not allowed:
      return {"status": "failed", "message": permission_msg}

    safe, msg = self.check_security(tool_name, arguments)
    if not safe:
      return {"status": "failed", "message": msg}

    if tool_name == "sys_query_vehicle_speed":
      return {"status": "success", "speed": self.vehicle_state["speed"]}

    if tool_name == "list_active_toolsets":
      ids = self.toolset_manager.get_active_toolset_ids()
      return {"status": "success", "active_toolsets": ids}

    handler = TOOL_HANDLERS.get(tool_name)
    if not handler:
      return {"status": "failed", "message": "未知工具"}
    return handler(self.vehicle_state, arguments)
