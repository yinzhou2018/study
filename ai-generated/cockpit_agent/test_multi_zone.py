"""多音区共享上下文单元测试"""
import io
import unittest
from contextlib import redirect_stdout

from cli_repl import CockpitRePL
from cockpit_agent import CockpitAgent
from tool_gateway import ToolGateway
from toolset_manager import ToolsetManager
from zone_context import parse_reply_prefix, parse_user_tag, tag_user_message


class FakeLLMClient:
  """记录消息并返回带音区前缀的固定回复"""

  def __init__(self):
    self.seen_messages = None

  def chat_stream(self, messages, tools, temperature=0.1, effort=None):
    self.seen_messages = messages
    yield {"type": "done",
           "message": {"role": "assistant", "content": "[zone=front_left] 好的"}}


class FakeAgent:
  """用于REPL测试的最小agent"""

  def __init__(self):
    self.effort = "high"
    self.messages = []
    self.last_zone = None
    self.last_input = None

  def interrupt(self):
    pass

  def chat_stream(self, user_input, **kwargs):
    self.last_zone = kwargs.get("zone_id")
    self.last_input = user_input
    return "[zone=front_left] 好的"


class ZoneContextTest(unittest.TestCase):

  def test_tag_user_message(self):
    """用户消息拼接为 [zone=xxx,user=guest] 前缀"""
    self.assertEqual(
        tag_user_message("有点闷", "front_left", "guest"),
        "[zone=front_left,user=guest] 有点闷"
    )

  def test_parse_user_tag(self):
    """解析用户消息前缀"""
    zone, user, text = parse_user_tag("[zone=front_right,user=guest] 把音量调大")
    self.assertEqual(zone, "front_right")
    self.assertEqual(user, "guest")
    self.assertEqual(text, "把音量调大")

  def test_parse_user_tag_no_prefix(self):
    """无前缀时返回原始文本"""
    zone, user, text = parse_user_tag("普通文本")
    self.assertIsNone(zone)
    self.assertIsNone(user)
    self.assertEqual(text, "普通文本")

  def test_parse_reply_prefix_zone(self):
    """解析 [zone=xxx] 回复前缀"""
    target, body = parse_reply_prefix("[zone=front_left] 已处理")
    self.assertEqual(target, "front_left")
    self.assertEqual(body, "已处理")

  def test_parse_reply_prefix_broadcast(self):
    """解析 [broadcast] 回复前缀"""
    target, body = parse_reply_prefix("[broadcast] 导航开始")
    self.assertEqual(target, "broadcast")
    self.assertEqual(body, "导航开始")

  def test_parse_reply_prefix_missing(self):
    """无前缀时返回None和原文，用于回退路由"""
    target, body = parse_reply_prefix("好的")
    self.assertIsNone(target)
    self.assertEqual(body, "好的")


class AgentSharedHistoryTest(unittest.TestCase):

  def test_chat_stream_tags_user_message(self):
    """chat_stream写入的用户消息带音区标签"""
    agent = CockpitAgent(llm_client=FakeLLMClient())
    reply = agent.chat_stream("有点闷", zone_id="front_left", user_id="guest")

    self.assertEqual(reply, "[zone=front_left] 好的")
    user_msg = agent.messages[1]
    self.assertEqual(user_msg["role"], "user")
    self.assertTrue(user_msg["content"].startswith("[zone=front_left,user=guest] "))

  def test_shared_history_across_zones(self):
    """多音区写入同一条历史"""
    agent = CockpitAgent(llm_client=FakeLLMClient())
    agent.chat_stream("有点闷", zone_id="front_left")
    agent.chat_stream("再开大一点", zone_id="front_right")

    user_msgs = [m for m in agent.messages if m["role"] == "user"]
    self.assertEqual(len(user_msgs), 2)
    self.assertTrue(user_msgs[0]["content"].startswith("[zone=front_left,user=guest] "))
    self.assertTrue(user_msgs[1]["content"].startswith("[zone=front_right,user=guest] "))


class ZoneAwareToolPermissionTest(unittest.TestCase):

  def setUp(self):
    self.tm = ToolsetManager()
    self.tm.load_toolsets(["toolset_media_entertainment", "toolset_body_control"])
    self.gateway = ToolGateway(self.tm)

  def test_passenger_can_use_media(self):
    """副驾可以调用普通媒体工具"""
    result = self.gateway.execute(
        "ctrl_music_play", {"category": "light"},
        requesting_zone="front_right"
    )
    self.assertEqual(result["status"], "success")

  def test_passenger_cannot_use_driver_only_tool(self):
    """副驾不能调用主驾专属工具"""
    result = self.gateway.execute(
        "ctrl_door_lock", {"status": "lock"},
        requesting_zone="front_right"
    )
    self.assertEqual(result["status"], "failed")
    self.assertIn("无权限", result["message"])


class HistoryCompressionTest(unittest.TestCase):

  def test_compression_preserves_zone_tags(self):
    """压缩摘要保留发起音区"""
    agent = CockpitAgent(llm_client=FakeLLMClient())
    agent.messages = [
        {"role": "system", "content": "system"},
        {"role": "user", "content": "[zone=front_left,user=guest] 有点闷"},
        {"role": "assistant", "content": None, "tool_calls": [{"id": "1", "function": {"name": "ctrl_window", "arguments": "{\"position\":\"front_left\",\"openness\":25}"}}]},
        {"role": "tool", "tool_call_id": "1", "content": '{"status":"success","message":"左前车窗已开"}'},
        {"role": "user", "content": "[zone=front_right,user=guest] 调低音量"},
        {"role": "assistant", "content": None, "tool_calls": [{"id": "2", "function": {"name": "ctrl_volume_media", "arguments": "{}"}}]},
        {"role": "tool", "tool_call_id": "2", "content": '{"status":"success","message":"音量已调低"}'},
        {"role": "assistant", "content": "已处理"},
        {"role": "assistant", "content": "状态正常"},
        {"role": "assistant", "content": "继续"},
        {"role": "assistant", "content": "结束"},
        {"role": "user", "content": "[zone=front_left,user=guest] 继续"},
    ]
    agent.compress_history()

    summary = agent.messages[1]["content"]
    self.assertIn("[front_left]", summary)
    self.assertIn("[front_right]", summary)


class ReplZoneTest(unittest.TestCase):

  def test_switch_zone(self):
    """/zone命令切换模拟音区"""
    repl = CockpitRePL(FakeAgent())
    self.assertEqual(repl.zone, "front_left")

    with redirect_stdout(io.StringIO()):
      repl._handle_command("/zone front_right")
    self.assertEqual(repl.zone, "front_right")

  def test_invalid_zone(self):
    """无效音区不变更"""
    repl = CockpitRePL(FakeAgent())
    buf = io.StringIO()
    with redirect_stdout(buf):
      repl._handle_command("/zone invalid")
    self.assertEqual(repl.zone, "front_left")
    self.assertIn("无效的音区", buf.getvalue())

  def test_chat_passes_zone_to_agent(self):
    """_chat把当前音区传给agent"""
    fake_agent = FakeAgent()
    repl = CockpitRePL(fake_agent)
    repl.zone = "front_right"

    buf = io.StringIO()
    with redirect_stdout(buf):
      repl._chat("把音量调大")

    self.assertEqual(fake_agent.last_zone, "front_right")
    self.assertIn("[目标音区: front_left]", buf.getvalue())


if __name__ == "__main__":
  unittest.main()
