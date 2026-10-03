"""effort命令行思考深度控制单元测试"""
import unittest

from llm_config import DEFAULT_EFFORT, build_effort_payload, get_provider
from cockpit_agent import CockpitAgent
from llm_client import MockLLMClient, OpenAICompatibleLLM


class BuildEffortPayloadTest(unittest.TestCase):

  def test_none_mode(self):
    """none模式返回enable_thinking=False"""
    payload = build_effort_payload("none")
    self.assertEqual(payload, {"enable_thinking": False})

  def test_low_mode(self):
    """low模式返回reasoning_effort=low"""
    payload = build_effort_payload("low")
    self.assertEqual(payload, {"reasoning_effort": "low"})

  def test_high_mode(self):
    """high模式返回reasoning_effort=high"""
    payload = build_effort_payload("high")
    self.assertEqual(payload, {"reasoning_effort": "high"})

  def test_max_mode(self):
    """max模式返回reasoning_effort=max"""
    payload = build_effort_payload("max")
    self.assertEqual(payload, {"reasoning_effort": "max"})

  def test_none_value_raises(self):
    """effort=None时抛出ValueError"""
    with self.assertRaises(ValueError):
      build_effort_payload(None)

  def test_invalid_mode_raises(self):
    """无效模式抛出ValueError"""
    with self.assertRaises(ValueError):
      build_effort_payload("invalid")


class ProviderEffortPayloadTest(unittest.TestCase):

  def test_voyah_payload(self):
    """voyah 供应商使用 enable_thinking / reasoning_effort 表达思考深度"""
    provider = "voyah"
    self.assertEqual(build_effort_payload("none", provider), {"enable_thinking": False})
    self.assertEqual(build_effort_payload("low", provider), {"reasoning_effort": "low"})
    self.assertEqual(build_effort_payload("high", provider), {"reasoning_effort": "high"})
    self.assertEqual(build_effort_payload("max", provider), {"reasoning_effort": "max"})

  def test_deepseek_payload(self):
    """deepseek 供应商所有effort均使用 reasoning_effort 表达思考深度"""
    provider = "deepseek"
    self.assertEqual(build_effort_payload("none", provider), {"reasoning_effort": "none"})
    self.assertEqual(build_effort_payload("low", provider), {"reasoning_effort": "low"})
    self.assertEqual(build_effort_payload("high", provider), {"reasoning_effort": "high"})
    self.assertEqual(build_effort_payload("max", provider), {"reasoning_effort": "max"})

  def test_provider_none_uses_default_provider(self):
    """provider=None 时使用默认供应商的映射"""
    self.assertEqual(build_effort_payload("high", None), {"reasoning_effort": "high"})

  def test_unknown_provider_falls_back(self):
    """未知供应商回退到默认供应商的映射"""
    self.assertEqual(build_effort_payload("high", "nonexistent"), {"reasoning_effort": "high"})

  def test_provider_profile_uses_registered_payload_function(self):
    """provider profile 中的 build_effort_payload 函数直接生效"""
    profile = get_provider("deepseek")
    self.assertEqual(build_effort_payload("high", profile),
                     {"reasoning_effort": "high"})

  def test_provider_profile_none_effort_raises(self):
    """profile 的映射函数对非法 effort 同样校验报错"""
    profile = get_provider("voyah")
    with self.assertRaises(ValueError):
      build_effort_payload("bogus", profile)


class OpenAICompatibleEffortTest(unittest.TestCase):

  def _capture_payload(self, method, effort, client=None):
    """辅助：拦截requests.post并返回payload"""
    client = client or OpenAICompatibleLLM(base_url="http://fake", api_key="fake")
    captured = {}

    class FakeResponse:
      def raise_for_status(self):
        pass

      def json(self):
        return {"choices": [{"message": {"role": "assistant", "content": "ok"}}]}

      def iter_lines(self, decode_unicode=True):
        yield 'data: [DONE]'

      def close(self):
        pass

    def fake_post(url, headers=None, json=None, timeout=None, stream=False):
      captured["payload"] = json
      return FakeResponse()

    import llm_client
    orig_post = llm_client.requests.post
    llm_client.requests.post = fake_post
    try:
      if method == "chat":
        client.chat([], [], effort=effort)
      else:
        list(client.chat_stream([], [], effort=effort))
    finally:
      llm_client.requests.post = orig_post
    return captured["payload"]

  def test_chat_includes_reasoning_effort_high(self):
    """chat() payload包含reasoning_effort=high"""
    payload = self._capture_payload("chat", "high")
    self.assertEqual(payload["reasoning_effort"], "high")

  def test_chat_includes_reasoning_effort_low(self):
    """chat() payload包含reasoning_effort=low"""
    payload = self._capture_payload("chat", "low")
    self.assertEqual(payload["reasoning_effort"], "low")

  def test_chat_none_mode_sets_enable_thinking_false(self):
    """chat() none模式设置enable_thinking=False且不含reasoning_effort"""
    payload = self._capture_payload("chat", "none")
    self.assertFalse(payload["enable_thinking"])
    self.assertNotIn("reasoning_effort", payload)

  def test_chat_stream_includes_reasoning_effort_max(self):
    """chat_stream() payload包含reasoning_effort=max"""
    payload = self._capture_payload("stream", "max")
    self.assertEqual(payload["reasoning_effort"], "max")

  def test_deepseek_chat_uses_reasoning_effort(self):
    """deepseek 供应商 client 的 payload 使用 reasoning_effort"""
    client = OpenAICompatibleLLM(base_url="http://fake", api_key="fake", provider_id="deepseek")
    payload = self._capture_payload("chat", "high", client=client)
    self.assertEqual(payload["reasoning_effort"], "high")

  def test_chat_stream_none_effort_raises(self):
    """chat_stream() effort=None时抛出ValueError"""
    client = OpenAICompatibleLLM(base_url="http://fake", api_key="fake")
    with self.assertRaises(ValueError):
      self._capture_payload("stream", None, client=client)


class MockClientEffortTest(unittest.TestCase):

  def test_mock_chat_accepts_effort(self):
    """MockLLMClient.chat()接受effort参数不报错"""
    client = MockLLMClient()
    response = client.chat(
        [{"role": "user", "content": "你好"}], [], effort="high")
    self.assertIn("choices", response)

  def test_mock_chat_stream_accepts_effort(self):
    """MockLLMClient.chat_stream()接受effort参数不报错"""
    client = MockLLMClient()
    events = list(client.chat_stream(
        [{"role": "user", "content": "你好"}], [], effort="low"))
    done_events = [e for e in events if e["type"] == "done"]
    self.assertEqual(len(done_events), 1)


class AgentEffortTest(unittest.TestCase):

  def test_default_effort_is_high(self):
    """CockpitAgent默认effort为high"""
    agent = CockpitAgent(llm_client=MockLLMClient())
    self.assertEqual(agent.effort, "high")
    self.assertEqual(agent.effort, DEFAULT_EFFORT)

  def test_effort_settable(self):
    """CockpitAgent.effort可设置"""
    agent = CockpitAgent(llm_client=MockLLMClient())
    agent.effort = "low"
    self.assertEqual(agent.effort, "low")

  def test_effort_passed_to_llm_chat(self):
    """chat()将effort传递给LLM客户端"""
    captured = {}

    class SpyLLMClient:
      def chat(self, messages, tools, temperature=0.1, effort=None):
        captured["effort"] = effort
        return {"choices": [{"message": {"role": "assistant", "content": "ok"}}]}

      def chat_stream(self, *args, **kwargs):
        pass

    agent = CockpitAgent(llm_client=SpyLLMClient())
    agent.effort = "max"
    agent.chat("你好", verbose=False)
    self.assertEqual(captured["effort"], "max")

  def test_effort_passed_to_llm_chat_stream(self):
    """chat_stream()将effort传递给LLM客户端"""
    captured = {}

    class SpyLLMClient:
      def chat(self, *args, **kwargs):
        pass

      def chat_stream(self, messages, tools, temperature=0.1, effort=None):
        captured["effort"] = effort
        yield {"type": "done", "message": {"role": "assistant", "content": "ok"}}

    agent = CockpitAgent(llm_client=SpyLLMClient())
    agent.effort = "none"
    agent.chat_stream("你好")
    self.assertEqual(captured["effort"], "none")


if __name__ == "__main__":
  unittest.main()
