"""空调总电源开关 ctrl_ac_power 单元测试"""
import unittest

from tool_gateway import ToolGateway
from toolset_manager import ToolsetManager


class AcPowerTest(unittest.TestCase):
  """覆盖 on/off 执行落库、message 文案、白名单校验"""

  def setUp(self):
    self.tm = ToolsetManager()
    self.tm.load_toolsets(["toolset_climate_control"])
    self.gateway = ToolGateway(self.tm)

  def test_turn_on(self):
    """开启空调：落库 ac_power=on，回填开启文案"""
    result = self.gateway.execute(
        "ctrl_ac_power", {"status": "on"}
    )
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["ac_power"], "on")
    self.assertIn("空调已开启", result["message"])

  def test_turn_off(self):
    """关闭空调：落库 ac_power=off，回填关闭文案"""
    self.gateway.vehicle_state["ac_power"] = "on"
    result = self.gateway.execute(
        "ctrl_ac_power", {"status": "off"}
    )
    self.assertEqual(result["status"], "success")
    self.assertEqual(self.gateway.vehicle_state["ac_power"], "off")
    self.assertIn("空调已关闭", result["message"])

  def test_default_state_is_off(self):
    """初始状态默认关闭"""
    self.assertEqual(self.gateway.vehicle_state["ac_power"], "off")

  def test_passenger_can_control(self):
    """副驾音区可调用空调开关（非主驾专属）"""
    result = self.gateway.execute(
        "ctrl_ac_power", {"status": "on"},
        requesting_zone="front_right"
    )
    self.assertEqual(result["status"], "success")

  def test_rejected_without_toolset_loaded(self):
    """未加载空调工具集时白名单拦截"""
    tm = ToolsetManager()  # 未加载任何业务工具集
    gateway = ToolGateway(tm)
    result = gateway.execute("ctrl_ac_power", {"status": "on"})
    self.assertEqual(result["status"], "failed")
    self.assertIn("白名单", result["message"])


if __name__ == "__main__":
  unittest.main()
