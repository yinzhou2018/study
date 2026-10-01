"""车窗控制合并工具 ctrl_window 单元测试"""
import unittest

from tool_gateway import ToolGateway, WINDOW_POSITION_MAP
from toolset_manager import ToolsetManager


class WindowControlTest(unittest.TestCase):
  """覆盖各位置执行、开度默认值、高速安全限制"""

  def setUp(self):
    self.tm = ToolsetManager()
    self.tm.load_toolsets(["toolset_body_control"])
    self.gateway = ToolGateway(self.tm)

  def test_each_position_updates_own_state(self):
    """各单独位置执行后落库对应状态键，回填中文标签"""
    cases = {
        "front_left": ("window_front_left", "左前车窗"),
        "front_right": ("window_front_right", "右前车窗"),
        "rear_left": ("window_rear_left", "左后车窗"),
        "rear_right": ("window_rear_right", "右后车窗"),
    }
    for position, (state_key, label) in cases.items():
      result = self.gateway.execute(
          "ctrl_window", {"position": position, "openness": 30}
      )
      self.assertEqual(result["status"], "success", msg=position)
      self.assertEqual(self.gateway.vehicle_state[state_key], 30, msg=position)
      self.assertIn(label, result["message"], msg=position)
      self.assertIn("30%", result["message"], msg=position)

  def test_all_position_updates_all_windows(self):
    """all 联动写入四个车窗状态键"""
    result = self.gateway.execute(
        "ctrl_window", {"position": "all", "openness": 100}
    )
    self.assertEqual(result["status"], "success")
    self.assertIn("全车车窗", result["message"])
    for state_key, _ in WINDOW_POSITION_MAP.values():
      self.assertEqual(self.gateway.vehicle_state[state_key], 100)

  def test_openness_defaults_to_50(self):
    """省略 openness 时默认 50%"""
    result = self.gateway.execute(
        "ctrl_window", {"position": "front_left"}
    )
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["window_front_left"], 50)
    self.assertIn("50%", result["message"])

  def test_low_speed_allows_full_open(self):
    """低速（默认42km/h）允许全开"""
    result = self.gateway.execute(
        "ctrl_window", {"position": "front_right", "openness": 100}
    )
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["window_front_right"], 100)

  def test_high_speed_blocks_openness_over_25(self):
    """高速（>60km/h）开度超过25%被拦截，状态不落库"""
    self.gateway.vehicle_state["speed"] = 80
    self.gateway.vehicle_state["window_front_left"] = 0
    result = self.gateway.execute(
        "ctrl_window", {"position": "front_left", "openness": 50}
    )
    self.assertEqual(result["status"], "failed")
    self.assertIn("25%", result["message"])
    # 拦截后状态不应被修改
    self.assertEqual(self.gateway.vehicle_state["window_front_left"], 0)

  def test_high_speed_allows_openness_25(self):
    """高速下开度恰好25%放行"""
    self.gateway.vehicle_state["speed"] = 80
    result = self.gateway.execute(
        "ctrl_window", {"position": "rear_left", "openness": 25}
    )
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["window_rear_left"], 25)

  def test_high_speed_blocks_all_position_too(self):
    """高速下 all 位置开度>25%同样被拦截"""
    self.gateway.vehicle_state["speed"] = 80
    result = self.gateway.execute(
        "ctrl_window", {"position": "all", "openness": 30}
    )
    self.assertEqual(result["status"], "failed")
    self.assertIn("25%", result["message"])


if __name__ == "__main__":
  unittest.main()
