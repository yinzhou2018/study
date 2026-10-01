# 多音区配置
AUDIO_ZONES = ("front_left", "front_right", "rear_left", "rear_right")
DEFAULT_ZONE = "front_left"
DEFAULT_USER_ID = "guest"


# 全程固定不变的System Prompt,最大化Prefix KV Cache收益
# 全量工具集列表由 ToolsetManager 从 toolsets.json 动态生成,此处仅保留模板
SYSTEM_PROMPT_TEMPLATE = """
## 角色定义
你是智能座舱车载助手,在持续对话中通过调用工具为用户提供车辆控制、信息查询、娱乐导航等服务。

## 核心规则
- 基于用户请求按需调用load_toolsets加载对应工具集。
- 只能使用当前已激活工具集内的工具,禁止使用未加载的能力,禁止自创工具和参数。
- 同一话题下的追问,复用已激活的工具集,不要重复加载。
- 工具需要位置参数而用户输入没有明确表达时用当前用户所在音区位置。

## 多音区多用户对话
- 用户消息以 [zone=xxx,user=yyy] 开头，表示发言音区和用户身份；未登录用户统一为 guest。
- 所有音区共享同一辆车状态和对话上下文，不要把每个音区当成独立会话。
- 后续发言可能来自不同音区，你需要自行判断是否承接之前的话题。
- 涉及个人隐私（通话、消息、日程）时，只对发起音区回复，不要跨区泄露。
- 回复以 [zone=xxx] 或 [broadcast] 开头，表示播报对象；缺省时由系统回退到发起音区。
- 多音区指令冲突时，优先级为：安全优先 > 主驾优先 > 发起者优先。
- 无法判断跨音区指代时，主动确认发起音区，不要猜测。

## 全量工具集
{toolset_listing}

## 回复要求
- 自然口语化,简洁清晰,适合开车时语音播报
- 不确定的操作直接询问用户,不要猜测
- 思考与回复均使用简体中文
"""


def build_system_prompt(toolset_listing: str) -> str:
  """将工具集列表填入模板,生成完整System Prompt"""
  return SYSTEM_PROMPT_TEMPLATE.format(toolset_listing=toolset_listing)

# 系统工具定义（常驻）,toolset_ids 枚举由 ToolsetManager 从 toolsets.json 动态传入


def build_system_tools(toolset_ids: list) -> list:
  """根据全量工具集ID构建系统工具,确保 enum 与 toolsets.json 保持一致"""
  return [
      {
          "type": "function",
          "function": {
              "name": "load_toolsets",
              "description": "批量加载并激活指定的车载工具集,加载成功后可使用对应工具集内的所有业务工具。",
              "parameters": {
                  "type": "object",
                  "properties": {
                      "toolset_ids": {
                          "type": "array",
                          "items": {
                              "type": "string",
                              "enum": toolset_ids
                          },
                          "minItems": 1,
                          "maxItems": MAX_ACTIVE_TOOLSETS,
                          "description": f"要加载的工具集ID列表,最多传入{MAX_ACTIVE_TOOLSETS}个"
                      }
                  },
                  "required": ["toolset_ids"]
              }
          }
      },
      {
          "type": "function",
          "function": {
              "name": "list_active_toolsets",
              "description": "查询当前已激活的所有工具集及对应可用能力",
              "parameters": {
                  "type": "object",
                  "properties": {},
                  "required": []
              }
          }
      }
  ]


# 最大同时激活工具集数量
MAX_ACTIVE_TOOLSETS = 3

# 模拟车辆实时状态（真实环境从车机总线获取）
MOCK_VEHICLE_STATE = {
    # 行驶基础
    "speed": 42,  # km/h
    "outside_temp": 28,
    "inside_temp": 26,
    "range": 380,  # km
    "battery_soc": 80,  # %

    # 车身控制
    "window_front_left": 0,
    "window_front_right": 0,
    "window_rear_left": 0,
    "window_rear_right": 0,
    "sunroof_tilt": "off",
    "sunroof_open": 0,
    "sunroof_shade": 0,
    "door_lock": "lock",
    "child_lock": "off",
    "tailgate": "close",
    "door_handle_front_left": "retract",
    "door_handle_front_right": "retract",
    "mirror_fold": "unfold",
    "wiper_front": "off",
    "wiper_rear": "off",

    # 灯光照明
    "headlight_mode": "auto",
    "drl": "on",
    "fog_light": "off",
    "ambient_light_color": "white",
    "ambient_light_brightness": 50,
    "ambient_scene": "driving",
    "welcome_light": "off",
    "star_roof_brightness": 0,
    "star_roof_mode": "static",
    "interior_reading_front_left": "off",
    "interior_reading_front_right": "off",
    "interior_reading_rear": "off",

    # 座椅系统
    "seat_memory": 1,
    "seat_heat_front_left": 0,
    "seat_heat_front_right": 0,
    "seat_heat_rear": 0,
    "seat_vent_front_left": 0,
    "seat_vent_front_right": 0,
    "seat_vent_rear": 0,
    "seat_massage_front_left": "off",
    "seat_massage_front_right": "off",
    "seat_lumbar": 3,
    "seat_boss_key": "off",
    "seat_recline": 2,

    # 空调温控
    "ac_power": "off",
    "ac_temperature": 24,
    "ac_fan_speed": 2,
    "ac_circulation": "inner",
    "ac_vent_mode": "face",
    "air_purifier": "off",
    "ionizer": "off",

    # 座舱舒适
    "fridge_mode": "off",
    "fridge_temp": 5,
    "aroma_scent": "tea",
    "aroma_intensity": "low",
    "panoramic_shade": 0,
    "armrest_heat": "off",

    # 影音娱乐
    "music_category": None,
    "music_track": 0,
    "volume_media": 15,
    "sound_mode": "hifi",
    "radio_frequency": None,
    "media_mute": "off",
    "rear_entertainment": "off",

    # 导航
    "nav_active": False,
    "nav_destination": None,
    "nav_route_pref": "fastest",

    # 充电
    "charge_active": False,
    "charge_limit": 80,
    "charge_schedule_start": None,
    "charge_schedule_end": None,
    "v2l_discharge": "off",

    # 通讯
    "phone_call_state": None,
    "message_unread": 2,

    # 驾驶辅助
    "cruise_control": "off",
    "cruise_speed": None,
    "lane_keep": "off",
    "acc_distance": 2,
    "energy_recovery": "medium",
    "auto_park": None,
    "remote_park": None,
    "hud_display": "on",
}
