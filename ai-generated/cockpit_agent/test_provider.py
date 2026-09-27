"""多供应商注册与运行期切换单元测试"""
import io
import unittest
from contextlib import redirect_stdout

from cli_repl import CockpitRePL
from cockpit_agent import CockpitAgent
from llm_client import MockLLMClient, OpenAICompatibleLLM
from llm_config import DEFAULT_PROVIDER_ID, PROVIDERS, get_provider


REQUIRED_FIELDS = ("base_url", "api_key", "model", "build_effort_payload")


class ProviderRegistryTest(unittest.TestCase):

  def test_providers_non_empty(self):
    self.assertGreater(len(PROVIDERS), 0)

  def test_default_provider_is_registered(self):
    self.assertIn(DEFAULT_PROVIDER_ID, PROVIDERS)

  def test_profile_fields_complete(self):
    for provider_id, profile in PROVIDERS.items():
      for field in REQUIRED_FIELDS:
        with self.subTest(provider=provider_id, field=field):
          self.assertIn(field, profile)

  def test_get_provider_returns_profile(self):
    profile = get_provider("deepseek")
    self.assertEqual(profile["model"], "deepseek-flash")

  def test_get_provider_unknown_raises(self):
    with self.assertRaises(ValueError):
      get_provider("nonexistent")


class AgentSetProviderTest(unittest.TestCase):

  def test_switch_updates_provider_id(self):
    agent = CockpitAgent(llm_client=MockLLMClient())
    agent.set_provider("deepseek")
    self.assertEqual(agent.provider_id, "deepseek")

  def test_switch_invalid_raises_and_preserves_id(self):
    agent = CockpitAgent(llm_client=MockLLMClient(), provider_id="voyah")
    with self.assertRaises(ValueError):
      agent.set_provider("nonexistent")
    self.assertEqual(agent.provider_id, "voyah")

  def test_switch_preserves_history(self):
    agent = CockpitAgent(llm_client=MockLLMClient())
    agent.messages.append({"role": "user", "content": "你好"})
    before = list(agent.messages)
    agent.set_provider("deepseek")
    self.assertEqual(agent.messages, before)

  def test_switch_reconfigures_openai_client(self):
    client = OpenAICompatibleLLM(base_url="http://fake", api_key="fake", model="m1")
    agent = CockpitAgent(llm_client=client, provider_id="voyah")
    agent.set_provider("deepseek")
    self.assertEqual(agent.provider_id, "deepseek")
    self.assertEqual(client.model, "deepseek-flash")
    self.assertEqual(client.provider_id, "deepseek")
    # 归一化后 base_url 不含结尾 /v1，由客户端统一补 /v1/chat/completions
    self.assertEqual(client.base_url, "https://api.deepseek.com")

  def test_switch_invalid_preserves_client_config(self):
    client = OpenAICompatibleLLM(base_url="http://fake", api_key="fake", model="m1")
    agent = CockpitAgent(llm_client=client, provider_id="voyah")
    with self.assertRaises(ValueError):
      agent.set_provider("nonexistent")
    self.assertEqual(agent.provider_id, "voyah")
    self.assertEqual(client.model, "m1")


class ProviderCommandTest(unittest.TestCase):

  def _capture(self, agent, arg):
    repl = CockpitRePL(agent)
    buf = io.StringIO()
    with redirect_stdout(buf):
      repl._handle_provider(arg)
    return buf.getvalue()

  def test_check_current_provider(self):
    agent = CockpitAgent(llm_client=MockLLMClient())
    output = self._capture(agent, None)
    self.assertIn(agent.provider_id, output)
    for provider_id in PROVIDERS:
      self.assertIn(provider_id, output)

  def test_switch_provider_via_command(self):
    agent = CockpitAgent(llm_client=MockLLMClient())
    output = self._capture(agent, "deepseek")
    self.assertEqual(agent.provider_id, "deepseek")
    self.assertIn("deepseek", output)

  def test_invalid_provider_via_command(self):
    agent = CockpitAgent(llm_client=MockLLMClient(), provider_id="voyah")
    output = self._capture(agent, "nonexistent")
    self.assertIn("无效的供应商", output)
    self.assertEqual(agent.provider_id, "voyah")
