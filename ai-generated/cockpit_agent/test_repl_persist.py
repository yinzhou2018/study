"""REPL 切换 provider/effort 后持久化集成测试"""
import io
import unittest
from contextlib import redirect_stdout

import cli_repl
from cli_repl import CockpitRePL
from cockpit_agent import CockpitAgent
from llm_client import MockLLMClient


def _run_with_prefs_stub(agent, handler_name, arg):
  """调用指定handler,拦截update_pref避免污染真实文件,返回(calls, output)"""
  stub = []
  orig = cli_repl.update_pref

  def fake_update(key, value):
    stub.append((key, value))

  cli_repl.update_pref = fake_update
  buf = io.StringIO()
  try:
    with redirect_stdout(buf):
      getattr(CockpitRePL(agent), handler_name)(arg)
  finally:
    cli_repl.update_pref = orig
  return stub, buf.getvalue()


class EffortCommandPersistTest(unittest.TestCase):

  def test_switch_effort_persists(self):
    agent = CockpitAgent(llm_client=MockLLMClient())
    calls, _ = _run_with_prefs_stub(agent, "_handle_effort", "low")
    self.assertEqual(calls, [("effort", "low")])
    self.assertEqual(agent.effort, "low")

  def test_invalid_effort_not_persisted(self):
    agent = CockpitAgent(llm_client=MockLLMClient())
    calls, _ = _run_with_prefs_stub(agent, "_handle_effort", "invalid")
    self.assertEqual(calls, [])
    self.assertEqual(agent.effort, "high")

  def test_query_effort_not_persisted(self):
    agent = CockpitAgent(llm_client=MockLLMClient())
    calls, _ = _run_with_prefs_stub(agent, "_handle_effort", None)
    self.assertEqual(calls, [])

  def test_persist_failure_warns_but_switch_succeeds(self):
    agent = CockpitAgent(llm_client=MockLLMClient())
    orig = cli_repl.update_pref

    def raising(key, value):
      raise OSError("disk full")

    cli_repl.update_pref = raising
    buf = io.StringIO()
    try:
      with redirect_stdout(buf):
        CockpitRePL(agent)._handle_effort("low")
    finally:
      cli_repl.update_pref = orig
    self.assertEqual(agent.effort, "low")
    self.assertIn("偏好持久化失败", buf.getvalue())


class ProviderCommandPersistTest(unittest.TestCase):

  def test_switch_provider_persists(self):
    agent = CockpitAgent(llm_client=MockLLMClient())
    calls, _ = _run_with_prefs_stub(agent, "_handle_provider", "deepseek")
    self.assertEqual(calls, [("provider", "deepseek")])
    self.assertEqual(agent.provider_id, "deepseek")

  def test_invalid_provider_not_persisted(self):
    agent = CockpitAgent(llm_client=MockLLMClient(), provider_id="voyah")
    calls, _ = _run_with_prefs_stub(agent, "_handle_provider", "nonexistent")
    self.assertEqual(calls, [])
    self.assertEqual(agent.provider_id, "voyah")

  def test_query_provider_not_persisted(self):
    agent = CockpitAgent(llm_client=MockLLMClient())
    calls, _ = _run_with_prefs_stub(agent, "_handle_provider", None)
    self.assertEqual(calls, [])


if __name__ == "__main__":
  unittest.main()
