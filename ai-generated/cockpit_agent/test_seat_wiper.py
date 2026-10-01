"""座椅与雨刮合并工具单元测试：ctrl_seat_heat/vent/massage、ctrl_wiper"""
import unittest

from tool_gateway import ToolGateway
from toolset_manager import ToolsetManager


class SeatHeatTest(unittest.TestCase):

  def setUp(self):
    self.tm = ToolsetManager()
    self.tm.load_toolsets(["toolset_seat_system"])
    self.gateway = ToolGateway(self.tm)

  def test_each_position_heat(self):
    """各位置加热落库并回填中文标签"""
    cases = {
        "front_left": ("seat_heat_front_left", "主驾"),
        "front_right": ("seat_heat_front_right", "副驾"),
        "rear": ("seat_heat_rear", "后排"),
    }
    for position, (state_key, label) in cases.items():
      result = self.gateway.execute(
          "ctrl_seat_heat", {"position": position, "level": 2}
      )
      self.assertEqual(result["status"], "success", msg=position)
      self.assertEqual(self.gateway.vehicle_state[state_key], 2, msg=position)
      self.assertIn(label, result["message"], msg=position)

  def test_heat_level_zero_closed(self):
    """level=0 回填关闭文案"""
    result = self.gateway.execute(
        "ctrl_seat_heat", {"position": "front_left", "level": 0}
    )
    self.assertEqual(result["status"], "success")
    self.assertIn("主驾座椅加热已关闭", result["message"])


class SeatVentTest(unittest.TestCase):

  def setUp(self):
    self.tm = ToolsetManager()
    self.tm.load_toolsets(["toolset_seat_system"])
    self.gateway = ToolGateway(self.tm)

  def test_vent_rear(self):
    """后排通风落库"""
    result = self.gateway.execute(
        "ctrl_seat_vent", {"position": "rear", "level": 3}
    )
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["seat_vent_rear"], 3)
    self.assertIn("后排座椅通风已调至3档", result["message"])

  def test_vent_zero_closed(self):
    """level=0 回填关闭文案"""
    result = self.gateway.execute(
        "ctrl_seat_vent", {"position": "front_right", "level": 0}
    )
    self.assertIn("副驾座椅通风已关闭", result["message"])


class SeatMassageTest(unittest.TestCase):

  def setUp(self):
    self.tm = ToolsetManager()
    self.tm.load_toolsets(["toolset_seat_system"])
    self.gateway = ToolGateway(self.tm)

  def test_massage_on(self):
    """按摩开启落库 mode 并回填文案"""
    result = self.gateway.execute(
        "ctrl_seat_massage",
        {"position": "front_left", "mode": "waist", "level": 1}
    )
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["seat_massage_front_left"], "waist")
    self.assertIn("主驾座椅按摩waist模式1档", result["message"])

  def test_massage_off(self):
    """mode=off 回填关闭文案"""
    result = self.gateway.execute(
        "ctrl_seat_massage",
        {"position": "front_right", "mode": "off", "level": 1}
    )
    self.assertIn("副驾座椅按摩已关闭", result["message"])


class WiperTest(unittest.TestCase):

  def setUp(self):
    self.tm = ToolsetManager()
    self.tm.load_toolsets(["toolset_body_control"])
    self.gateway = ToolGateway(self.tm)

  def test_wiper_front_auto(self):
    """前雨刮 auto 档落库"""
    result = self.gateway.execute(
        "ctrl_wiper", {"position": "front", "level": "auto"}
    )
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["wiper_front"], "auto")
    self.assertIn("前雨刮已调至auto档", result["message"])

  def test_wiper_rear_off(self):
    """后雨刮 off 回填关闭文案"""
    result = self.gateway.execute(
        "ctrl_wiper", {"position": "rear", "level": "off"}
    )
    self.assertEqual(self.gateway.vehicle_state["wiper_rear"], "off")
    self.assertIn("后雨刮已关闭", result["message"])


class WhitelistTest(unittest.TestCase):

  def test_seat_heat_rejected_without_toolset(self):
    """未加载座椅工具集时白名单拦截"""
    tm = ToolsetManager()
    gateway = ToolGateway(tm)
    result = gateway.execute(
        "ctrl_seat_heat", {"position": "front_left", "level": 2}
    )
    self.assertEqual(result["status"], "failed")
    self.assertIn("白名单", result["message"])

  def test_wiper_rejected_without_toolset(self):
    """未加载车身工具集时雨刮白名单拦截"""
    tm = ToolsetManager()
    gateway = ToolGateway(tm)
    result = gateway.execute(
        "ctrl_wiper", {"position": "front", "level": "1"}
    )
    self.assertEqual(result["status"], "failed")
    self.assertIn("白名单", result["message"])


if __name__ == "__main__":
  unittest.main()
