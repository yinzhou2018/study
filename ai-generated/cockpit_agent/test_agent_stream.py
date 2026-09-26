"""CockpitAgent流式对话单元测试"""
import unittest

from cockpit_agent import CockpitAgent
from llm_client import MockLLMClient


class AgentStreamTest(unittest.TestCase):

  def test_chat_stream_text_reply(self):
    """流式对话返回最终回复并触发on_content和on_done"""
    agent = CockpitAgent(llm_client=MockLLMClient())
    tokens = []
    done_replies = []

    reply = agent.chat_stream(
        "你好",
        on_content=lambda t: tokens.append(t),
        on_done=lambda r: done_replies.append(r),
    )

    self.assertEqual(reply, "好的，已处理。")
    self.assertGreater(len(tokens), 0)
    self.assertEqual("".join(tokens), reply)
    self.assertEqual(done_replies, [reply])

  def test_chat_stream_tool_call_events(self):
    """流式对话触发on_tool_call和on_tool_result回调"""
    agent = CockpitAgent(llm_client=MockLLMClient())
    tool_calls = []
    tool_results = []

    reply = agent.chat_stream(
        "有点闷",
        on_tool_call=lambda n, a: tool_calls.append((n, a)),
        on_tool_result=lambda n, r: tool_results.append((n, r)),
    )

    # Mock的toolset ID不匹配toolsets.json，但工具调用事件仍应被触发
    self.assertGreater(len(tool_calls), 0)
    self.assertGreater(len(tool_results), 0)
    self.assertEqual(len(tool_calls), len(tool_results))
    # 第一个工具调用应为load_toolsets
    self.assertEqual(tool_calls[0][0], "load_toolsets")

  def test_interrupt_mid_stream(self):
    """生成期间设置打断标志后抛出InterruptedError"""
    agent = CockpitAgent(llm_client=MockLLMClient())
    tokens = []

    def on_content(text):
      tokens.append(text)
      agent.interrupt()

    with self.assertRaises(InterruptedError):
      agent.chat_stream("你好", on_content=on_content)

    # 第一个token已收到，第二个token前被打断
    self.assertEqual(len(tokens), 1)

  def test_interrupt_preserves_completed_work(self):
    """打断后已完成的tool结果保留在messages中"""
    agent = CockpitAgent(llm_client=MockLLMClient())
    call_count = 0

    def on_tool_result(name, result):
      nonlocal call_count
      call_count += 1
      if call_count >= 1:
        agent.interrupt()

    with self.assertRaises(InterruptedError):
      agent.chat_stream("有点闷", on_tool_result=on_tool_result)

    # 检查messages中至少有一个tool角色消息
    tool_msgs = [m for m in agent.messages if m["role"] == "tool"]
    self.assertGreaterEqual(len(tool_msgs), 1)

  def test_chat_backward_compatible(self):
    """chat()方法签名和行为保持不变"""
    agent = CockpitAgent(llm_client=MockLLMClient())

    # chat()应返回字符串
    reply = agent.chat("你好", verbose=False)
    self.assertIsInstance(reply, str)
    self.assertEqual(reply, "好的，已处理。")

    # verbose=True不报错
    agent2 = CockpitAgent(llm_client=MockLLMClient())
    reply2 = agent2.chat("你好", verbose=True)
    self.assertIsInstance(reply2, str)

  def test_chat_stream_resets_interrupt_flag(self):
    """chat_stream开始时重置打断标志"""
    agent = CockpitAgent(llm_client=MockLLMClient())
    agent._interrupted = True

    # chat_stream应重置标志并正常完成
    reply = agent.chat_stream("你好")
    self.assertEqual(reply, "好的，已处理。")
    self.assertFalse(agent._interrupted)

  def test_chat_stream_reasoning_callback(self):
    """流式对话触发on_reasoning回调，思考内容在回复内容之前"""
    agent = CockpitAgent(llm_client=MockLLMClient())
    reasoning_texts = []
    token_texts = []
    call_order = []

    def on_reasoning(text):
      reasoning_texts.append(text)
      call_order.append("reasoning")

    def on_content(text):
      token_texts.append(text)
      call_order.append("token")

    reply = agent.chat_stream(
        "你好",
        on_reasoning=on_reasoning,
        on_content=on_content,
    )

    self.assertEqual(reply, "好的，已处理。")
    self.assertGreater(len(reasoning_texts), 0)
    self.assertGreater(len(token_texts), 0)

    # reasoning应在token之前调用
    self.assertEqual(call_order[0], "reasoning")

    # 拼接reasoning文本应包含用户输入
    full_reasoning = "".join(reasoning_texts)
    self.assertIn("你好", full_reasoning)

  def test_chat_stream_reasoning_before_tool_call(self):
    """工具调用场景下思考内容仍在工具调用事件之前"""
    agent = CockpitAgent(llm_client=MockLLMClient())
    reasoning_texts = []
    tool_calls = []
    call_order = []

    def on_reasoning(text):
      reasoning_texts.append(text)
      call_order.append("reasoning")

    def on_tool_call(name, args):
      tool_calls.append((name, args))
      call_order.append("tool_call")

    agent.chat_stream(
        "有点闷",
        on_reasoning=on_reasoning,
        on_tool_call=on_tool_call,
    )

    self.assertGreater(len(reasoning_texts), 0)
    self.assertGreater(len(tool_calls), 0)
    # reasoning应在tool_call之前
    self.assertEqual(call_order[0], "reasoning")


if __name__ == "__main__":
  unittest.main()
