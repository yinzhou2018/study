"""全量工具执行覆盖测试：131 工具 happy path + 状态自洽 + trigger 联动"""
import json
import unittest

from tool_gateway import ToolGateway
from toolset_manager import ToolsetManager

TOOLSETS = json.load(open("toolsets.json"))

# 每个工具的合法调用参数（覆盖各枚举值）
PARAMS = {
    # 车身控制
    "ctrl_window": {"position": "front_left", "openness": 30},
    "ctrl_sunroof_tilt": {"status": "on"},
    "ctrl_sunroof_open": {"openness": 50},
    "ctrl_sunroof_shade": {"openness": 50},
    "ctrl_door_lock": {"status": "unlock"},
    "ctrl_child_lock": {"status": "on"},
    "ctrl_tailgate": {"action": "open"},
    "ctrl_door_soft_close": {"position": "front_left"},
    "ctrl_door_handle": {"position": "front_left", "status": "pop"},
    "ctrl_mirror_fold": {"status": "fold"},
    "ctrl_wiper": {"position": "front", "level": "1"},
    "query_window_status": {},
    "query_sunroof_tilt": {},
    "query_sunroof_open": {},
    "query_sunroof_shade": {},
    "query_door_lock_status": {},
    "query_child_lock_status": {},
    "query_tailgate_status": {},
    "query_door_handle_status": {},
    "query_mirror_fold_status": {},
    "query_wiper_status": {},
    # 灯光照明
    "ctrl_headlight_mode": {"mode": "low_beam"},
    "ctrl_drl": {"status": "on"},
    "ctrl_fog_light": {"status": "on"},
    "ctrl_ambient_light": {"color": "blue", "brightness": 80},
    "ctrl_ambient_scene": {"scene": "rest"},
    "ctrl_welcome_light": {"status": "on"},
    "ctrl_star_roof": {"brightness": 60, "mode": "breathe"},
    "ctrl_interior_reading": {"position": "front_left", "status": "on"},
    "query_headlight_mode": {},
    "query_drl_status": {},
    "query_fog_light_status": {},
    "query_ambient_light_status": {},
    "query_ambient_scene": {},
    "query_welcome_light_status": {},
    "query_star_roof_status": {},
    "query_reading_light_status": {},
    # 座椅系统
    "ctrl_seat_driver_memory": {"slot": 2},
    "ctrl_seat_heat": {"position": "front_left", "level": 2},
    "ctrl_seat_vent": {"position": "rear", "level": 1},
    "ctrl_seat_massage": {"position": "front_left", "mode": "waist", "level": 2},
    "ctrl_seat_driver_lumbar": {"level": 4},
    "ctrl_seat_boss_key": {"status": "on"},
    "ctrl_seat_rear_recline": {"angle": 3},
    "query_seat_heat_status": {},
    "query_seat_vent_status": {},
    "query_seat_massage_status": {},
    "query_seat_lumbar_status": {},
    "query_seat_boss_key_status": {},
    "query_seat_recline_status": {},
    # 空调温控
    "ctrl_ac_power": {"status": "on"},
    "ctrl_ac_temperature": {"temperature": 22},
    "ctrl_ac_fan_speed": {"level": 3},
    "ctrl_ac_circulation": {"mode": "outer"},
    "ctrl_ac_vent_mode": {"mode": "foot"},
    "ctrl_air_purifier": {"status": "on"},
    "ctrl_ionizer": {"status": "on"},
    "query_air_quality": {},
    "query_ac_power_status": {},
    "query_ac_temperature": {},
    "query_ac_fan_speed": {},
    "query_ac_circulation": {},
    "query_ac_vent_mode": {},
    "query_air_purifier_status": {},
    "query_ionizer_status": {},
    # 座舱舒适
    "ctrl_fridge": {"mode": "cold"},
    "ctrl_fridge_temp": {"temperature": 3},
    "ctrl_aroma_system": {"scent": "wood", "intensity": "high"},
    "ctrl_panoramic_shade": {"openness": 50},
    "ctrl_armrest_heat": {"status": "on"},
    "query_fridge_status": {},
    "query_fridge_temp": {},
    "query_aroma_status": {},
    "query_panoramic_shade_status": {},
    "query_armrest_heat_status": {},
    # 影音娱乐
    "ctrl_music_play": {"category": "pop"},
    "ctrl_music_next": {},
    "ctrl_music_prev": {},
    "ctrl_volume_media": {"level": 20},
    "ctrl_sound_mode": {"mode": "vocal"},
    "ctrl_radio_tune": {"frequency": "FM97.4"},
    "ctrl_media_mute": {"status": "on"},
    "ctrl_rear_entertainment": {"status": "on"},
    "query_music_status": {},
    "query_volume_media": {},
    "query_sound_mode_status": {},
    "query_radio_status": {},
    "query_rear_entertainment_status": {},
    # 导航出行
    "ctrl_nav_start": {"destination": "公司"},
    "ctrl_nav_stop": {},
    "ctrl_nav_route_pref": {"mode": "shortest"},
    "query_poi_nearby": {"keyword": "加油站", "type": "gas"},
    "query_traffic_status": {},
    "query_charging_station": {},
    "query_route_charge_plan": {"soc_threshold": 20},
    "query_nav_status": {},
    # 车辆信息
    "query_battery_status": {},
    "query_range_mileage": {},
    "query_energy_consumption": {},
    "query_tire_pressure": {},
    # 充电管理
    "ctrl_charge_start": {},
    "ctrl_charge_stop": {},
    "ctrl_charge_limit": {"percent": 90},
    "ctrl_charge_schedule": {"start_time": "23:00", "end_time": "06:00"},
    "ctrl_v2l_discharge": {"status": "on"},
    "query_charge_status": {},
    # 通讯通话
    "ctrl_phone_call": {"contact": "张三"},
    "ctrl_phone_answer": {},
    "ctrl_phone_hangup": {},
    "query_contact_search": {"keyword": "张"},
    "ctrl_message_read": {},
    # 驾驶辅助
    "ctrl_cruise_control": {"status": "on", "speed": 100},
    "ctrl_lane_keep": {"status": "on"},
    "ctrl_acc_distance": {"level": 3},
    "ctrl_energy_recovery": {"level": "high"},
    "ctrl_auto_park": {"mode": "vertical"},
    "ctrl_remote_park": {"direction": "forward"},
    "ctrl_hud_display": {"status": "off"},
    "query_cruise_control_status": {},
    "query_lane_keep_status": {},
    "query_acc_distance_status": {},
    "query_energy_recovery_status": {},
    "query_hud_status": {},
    # 座舱模式
    "trigger_car_wash_mode": {},
    "trigger_rest_mode": {},
    "trigger_commute_mode": {},
    "trigger_child_mode": {},
    "trigger_charge_mode": {},
    "trigger_camp_mode": {},
    "trigger_pet_mode": {},
    "trigger_stealth_mode": {},
}


class FullCoverageTest(unittest.TestCase):
  """131 工具全部执行成功（每工具集独立加载，规避 LRU 上限）"""

  def test_all_tools_success(self):
    for toolset_id, cfg in TOOLSETS.items():
      tm = ToolsetManager()
      tm.load_toolsets([toolset_id])
      gateway = ToolGateway(tm)
      for tool in cfg["tools"]:
        name = tool["function"]["name"]
        with self.subTest(toolset=toolset_id, tool=name):
          result = gateway.execute(name, PARAMS[name])
          self.assertEqual(
              result["status"], "success",
              f"{name} 执行失败: {result}")


class StateConsistencyTest(unittest.TestCase):
  """控制类落库后，查询类能读到当前状态（闭环验证）"""

  def _gateway_with(self, *toolsets):
    tm = ToolsetManager()
    tm.load_toolsets(list(toolsets))
    return ToolGateway(tm)

  def test_control_then_query_charge(self):
    """ctrl_charge_start 开启充电后 query_battery_status 读到新值"""
    gateway = self._gateway_with("toolset_charging_management",
                                 "toolset_vehicle_info")
    gateway.execute("ctrl_charge_start", {})
    result = gateway.execute("query_battery_status", {})
    self.assertEqual(result["charging"], True)

  def test_control_then_query_window(self):
    """ctrl_window 改变后 query_window_status 读到新值"""
    gateway = self._gateway_with("toolset_body_control")
    gateway.execute("ctrl_window", {"position": "front_left", "openness": 30})
    result = gateway.execute("query_window_status", {})
    self.assertEqual(result["front_left"], 30)

  def test_control_then_query_ac_temperature(self):
    """ctrl_ac_temperature 改变后 query_ac_temperature 读到新值"""
    gateway = self._gateway_with("toolset_climate_control")
    gateway.execute("ctrl_ac_temperature", {"temperature": 22})
    result = gateway.execute("query_ac_temperature", {})
    self.assertEqual(result["temperature"], 22)

  def test_control_then_query_seat_massage(self):
    """ctrl_seat_massage 改变后 query_seat_massage_status 读到模式与强度"""
    gateway = self._gateway_with("toolset_seat_system")
    gateway.execute("ctrl_seat_massage",
                    {"position": "front_left", "mode": "waist", "level": 2})
    result = gateway.execute("query_seat_massage_status", {})
    self.assertEqual(result["front_left"]["mode"], "waist")
    self.assertEqual(result["front_left"]["level"], 2)

  def test_control_then_query_nav(self):
    """ctrl_nav_start 后 query_nav_status 读到目的地"""
    gateway = self._gateway_with("toolset_navigation")
    gateway.execute("ctrl_nav_start", {"destination": "公司"})
    result = gateway.execute("query_nav_status", {})
    self.assertEqual(result["active"], True)
    self.assertEqual(result["destination"], "公司")


class TriggerLinkageTest(unittest.TestCase):
  """trigger 模式真实联动改变设备状态"""

  def setUp(self):
    self.tm = ToolsetManager()
    self.tm.load_toolsets(["toolset_cabin_modes", "toolset_body_control",
                           "toolset_climate_control"])
    self.gateway = ToolGateway(self.tm)

  def test_car_wash_mode_closes_windows_and_folds_mirror(self):
    """洗车模式联动关窗、关天窗、折后视镜"""
    self.gateway.vehicle_state["window_front_left"] = 50
    result = self.gateway.execute("trigger_car_wash_mode", {})
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["window_front_left"], 0)
    self.assertEqual(self.gateway.vehicle_state["sunroof_open"], 0)
    self.assertEqual(self.gateway.vehicle_state["mirror_fold"], "fold")

  def test_rest_mode_links_climate(self):
    """休息模式联动空调温度"""
    result = self.gateway.execute("trigger_rest_mode", {})
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["ac_temperature"], 24)
    self.assertEqual(self.gateway.vehicle_state["sunroof_tilt"], "on")

  def test_child_mode_locks_rear_windows(self):
    """儿童模式联动儿童锁与后排关窗"""
    result = self.gateway.execute("trigger_child_mode", {})
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["child_lock"], "on")
    self.assertEqual(self.gateway.vehicle_state["window_rear_left"], 0)
    self.assertEqual(self.gateway.vehicle_state["window_rear_right"], 0)


class StatePersistTest(unittest.TestCase):
  """关键控制类状态落库抽样验证"""

  def setUp(self):
    self.tm = ToolsetManager()
    self.tm.load_toolsets(["toolset_lighting_control", "toolset_seat_system",
                           "toolset_media_entertainment"])
    self.gateway = ToolGateway(self.tm)

  def test_lighting_state_persisted(self):
    """灯光控制落库"""
    self.gateway.execute("ctrl_headlight_mode", {"mode": "high_beam"})
    self.gateway.execute("ctrl_ambient_light", {"color": "red", "brightness": 90})
    self.assertEqual(self.gateway.vehicle_state["headlight_mode"], "high_beam")
    self.assertEqual(self.gateway.vehicle_state["ambient_light_color"], "red")
    self.assertEqual(self.gateway.vehicle_state["ambient_light_brightness"], 90)

  def test_ambient_light_64_color_persisted(self):
    """氛围灯64色中新增命名色落库"""
    self.gateway.execute("ctrl_ambient_light", {"color": "crimson", "brightness": 40})
    self.assertEqual(self.gateway.vehicle_state["ambient_light_color"], "crimson")
    self.assertEqual(self.gateway.vehicle_state["ambient_light_brightness"], 40)

  def test_seat_state_persisted(self):
    """座椅控制落库，可在当前值基础上再调整"""
    self.gateway.execute("ctrl_seat_heat", {"position": "front_left", "level": 1})
    self.assertEqual(self.gateway.vehicle_state["seat_heat_front_left"], 1)
    self.gateway.execute("ctrl_seat_heat", {"position": "front_left", "level": 3})
    self.assertEqual(self.gateway.vehicle_state["seat_heat_front_left"], 3)

  def test_media_state_persisted(self):
    """媒体控制落库，next/prev 在当前曲目基础上叠加"""
    self.gateway.execute("ctrl_music_play", {"category": "pop"})
    self.gateway.execute("ctrl_music_next", {})
    self.gateway.execute("ctrl_music_next", {})
    self.assertEqual(self.gateway.vehicle_state["music_track"], 3)
    self.gateway.execute("ctrl_music_prev", {})
    self.assertEqual(self.gateway.vehicle_state["music_track"], 2)


class AmbientLightColorEnumTest(unittest.TestCase):
  """氛围灯颜色枚举：64色、保留原有锚点色、移除 custom、无重复"""

  @classmethod
  def _colors(cls):
    tool = next(t for t in TOOLSETS["toolset_lighting_control"]["tools"]
                if t["function"]["name"] == "ctrl_ambient_light")
    return tool["function"]["parameters"]["properties"]["color"]["enum"]

  def test_color_enum_has_64_unique_values(self):
    colors = self._colors()
    self.assertEqual(len(colors), 64)
    self.assertEqual(len(set(colors)), 64)

  def test_original_anchors_kept_and_custom_removed(self):
    colors = set(self._colors())
    for anchor in ("white", "blue", "red", "green"):
      self.assertIn(anchor, colors)
    self.assertNotIn("custom", colors)


if __name__ == "__main__":
  unittest.main()
