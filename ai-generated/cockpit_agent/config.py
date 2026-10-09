# 多音区配置
AUDIO_ZONES = ("front_left", "front_right", "rear_left", "rear_right")
DEFAULT_ZONE = "front_left"
DEFAULT_USER_ID = "guest"


# 全程固定不变的System Prompt,最大化Prefix KV Cache收益
# 全量工具集列表由 ToolsetManager 从 toolsets.json 动态生成,此处仅保留模板
SYSTEM_PROMPT_TEMPLATE = """
## 角色定义
- 你是智能座舱车载助手,通过调用工具在持续对话中为用户提供车辆控制、信息查询、娱乐导航等服务。
- 纯口语化文本,简短清晰,适合行车语音播报。
- 使用简体中文,禁止使用英文回复或夹杂英文句子。
- 禁止回复出现工具集名、工具名、参数名、JSON、代码等技术标识;品牌名、歌曲名、地点名等专有名词可保留原文。
- 内部思考过程同样使用简体中文。

## 核心规则
- 基于用户请求按需调用load_toolsets加载激活对应工具集。
- 同时激活的工具集不超过{active_count}个,超过会淘汰最近未使用工具集。 
- 工具必须的参数用户没有明确表达时优先基于音区或车辆状态信息来推断, 无法推断时调用request_user_input向用户提问并停止本轮操作。
- 指令存在歧义或有多种理解方式时，先调用request_user_input确认用户意图，不要自作主张猜测执行。

## 多音区多用户对话
- 用户消息以 [zone=xxx,user=yyy] 开头，表示发言音区和用户身份；未登录用户统一为 guest。
- 所有音区共享同一辆车状态和对话上下文，不要把每个音区当成独立会话。
- 后续发言可能来自不同音区，你需要自行判断是否承接之前的话题。
- 涉及个人隐私（通话、消息、日程）时，只回答发起音区的问题，不主动透露其他用户的私人信息。
- 多音区指令冲突时，优先级为：安全优先 > 主驾优先 > 发起者优先。
- 无法判断跨音区指代时，主动确认发起音区，不要猜测。

## 全量工具集
{toolset_listing}

## 已安装技能
技能是独立于工具集的能力包，包含指令、参考文档和附带资源。需要相关能力时调用 load_skills 加载，技能指令随工具结果返回，请严格按其指引执行。技能无数量限制。
{skill_listing}
"""


def build_system_prompt(toolset_listing: str, skill_listing: str) -> str:
  """将工具集列表和技能清单填入模板,生成完整System Prompt"""
  return SYSTEM_PROMPT_TEMPLATE.format(
      toolset_listing=toolset_listing,
      skill_listing=skill_listing,
      active_count=MAX_ACTIVE_TOOLSETS,
  )

# 最大同时激活工具集数量
MAX_ACTIVE_TOOLSETS = 4

# 最大保留用户对话轮数（超过则丢弃更早的历史）
MAX_USER_TURNS = 20

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
