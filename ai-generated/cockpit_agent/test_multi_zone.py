"""多音区共享上下文单元测试"""
import io
import unittest
from contextlib import redirect_stdout

from cli_repl import CockpitRePL
from cockpit_agent import CockpitAgent
from tool_gateway import ToolGateway
from toolset_manager import ToolsetManager
from zone_context import parse_user_tag, tag_user_message


class FakeLLMClient:
  """记录消息并返回固定回复"""

  def __init__(self):
    self.seen_messages = None

  def chat_stream(self, messages, tools, temperature=0.1, effort=None):
    self.seen_messages = messages
    yield {"type": "done",
           "message": {"role": "assistant", "content": "好的"}}


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
    return "好的"


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


class AgentSharedHistoryTest(unittest.TestCase):

  def test_chat_stream_tags_user_message(self):
    """chat_stream写入的用户消息带音区标签"""
    agent = CockpitAgent(llm_client=FakeLLMClient())
    reply = agent.chat_stream("有点闷", zone_id="front_left", user_id="guest")

    self.assertEqual(reply, "好的")
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


class HistoryTrimTest(unittest.TestCase):
  """历史裁剪：仅保留最近 MAX_USER_TURNS 轮用户对话"""

  def _build_messages(self, n_turns, with_tool_calls=False):
    msgs = [{"role": "system", "content": "system"}]
    for i in range(n_turns):
      msgs.append({"role": "user",
                   "content": f"[zone=front_left,user=guest] q{i}"})
      if with_tool_calls:
        msgs.append({"role": "assistant", "content": None,
                     "tool_calls": [{"id": str(i),
                                     "function": {"name": "query", "arguments": "{}"}}]})
        msgs.append({"role": "tool", "tool_call_id": str(i),
                     "content": '{"status":"success"}'})
      msgs.append({"role": "assistant", "content": f"a{i}"})
    return msgs

  def test_keeps_recent_20_user_turns(self):
    """超过20轮时只保留最近20轮，System Prompt 始终保留"""
    agent = CockpitAgent(llm_client=FakeLLMClient())
    agent.messages = self._build_messages(25)
    agent._trim_history()
    user_msgs = [m for m in agent.messages if m["role"] == "user"]
    self.assertEqual(len(user_msgs), 20)
    self.assertIn("q5", user_msgs[0]["content"])
    self.assertIn("q24", user_msgs[-1]["content"])
    self.assertEqual(agent.messages[0]["role"], "system")

  def test_no_trim_under_20_turns(self):
    """不超过20轮时历史原样保留"""
    agent = CockpitAgent(llm_client=FakeLLMClient())
    before = self._build_messages(20)
    agent.messages = before
    agent._trim_history()
    self.assertEqual(agent.messages, before)

  def test_trim_cuts_at_user_boundary(self):
    """裁剪点在 user 消息边界，不产生孤儿 tool/assistant 消息"""
    agent = CockpitAgent(llm_client=FakeLLMClient())
    agent.messages = self._build_messages(25, with_tool_calls=True)
    agent._trim_history()
    self.assertEqual(agent.messages[1]["role"], "user")
    user_msgs = [m for m in agent.messages if m["role"] == "user"]
    self.assertEqual(len(user_msgs), 20)

  def test_chat_stream_auto_trims(self):
    """chat_stream 追加新用户消息后自动裁剪"""
    agent = CockpitAgent(llm_client=FakeLLMClient())
    agent.messages = self._build_messages(25)
    agent.chat_stream("q25")
    user_msgs = [m for m in agent.messages if m["role"] == "user"]
    self.assertEqual(len(user_msgs), 20)


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


if __name__ == "__main__":
  unittest.main()
