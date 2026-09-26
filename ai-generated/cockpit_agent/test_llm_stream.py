"""LLM流式输出单元测试"""
import json
import unittest

from llm_client import MockLLMClient, OpenAICompatibleLLM


class FakeSSEResponse:
  """模拟requests流式响应"""

  def __init__(self, lines):
    self._lines = lines
    self._closed = False

  def raise_for_status(self):
    pass

  def iter_lines(self, decode_unicode=True):
    for line in self._lines:
      yield line

  def close(self):
    self._closed = True


class MockClientStreamTest(unittest.TestCase):

  def test_text_response_streaming(self):
    """MockLLMClient对纯文本回复逐token流式输出"""
    client = MockLLMClient()
    messages = [{"role": "user", "content": "你好"}]
    tools = []

    events = list(client.chat_stream(messages, tools))

    content_events = [e for e in events if e["type"] == "content"]
    done_events = [e for e in events if e["type"] == "done"]

    self.assertGreater(len(content_events), 0)
    self.assertEqual(len(done_events), 1)

    # 拼接content应为完整文本
    full_text = "".join(e["text"] for e in content_events)
    self.assertEqual(full_text, done_events[0]["message"]["content"])
    # done事件的message不应有tool_calls
    self.assertIsNone(done_events[0]["message"].get("tool_calls"))

  def test_tool_call_response_no_content(self):
    """MockLLMClient对工具调用不产生content事件"""
    client = MockLLMClient()
    messages = [{"role": "user", "content": "有点闷"}]
    tools = []  # 无业务工具，触发load_toolsets

    events = list(client.chat_stream(messages, tools))

    reasoning_events = [e for e in events if e["type"] == "reasoning"]
    content_events = [e for e in events if e["type"] == "content"]
    done_events = [e for e in events if e["type"] == "done"]

    self.assertGreater(len(reasoning_events), 0)
    self.assertEqual(len(content_events), 0)
    self.assertEqual(len(done_events), 1)
    self.assertIsNotNone(done_events[0]["message"].get("tool_calls"))

  def test_reasoning_events_streamed(self):
    """MockLLMClient流式输出思考内容，reasoning事件在content事件之前"""
    client = MockLLMClient()
    messages = [{"role": "user", "content": "你好"}]
    tools = []

    events = list(client.chat_stream(messages, tools))

    reasoning_events = [e for e in events if e["type"] == "reasoning"]
    content_events = [e for e in events if e["type"] == "content"]
    done_events = [e for e in events if e["type"] == "done"]

    self.assertGreater(len(reasoning_events), 0)
    self.assertGreater(len(content_events), 0)

    # reasoning事件应在content事件之前
    first_reasoning_idx = events.index(reasoning_events[0])
    first_content_idx = events.index(content_events[0])
    self.assertLess(first_reasoning_idx, first_content_idx)

    # 拼接reasoning文本应为完整思考内容
    full_reasoning = "".join(e["text"] for e in reasoning_events)
    self.assertIn("你好", full_reasoning)


class OpenAICompatibleStreamTest(unittest.TestCase):

  def test_sse_content_parsing(self):
    """解析SSE delta content"""
    sse_lines = [
        'data: {"choices":[{"delta":{"content":"Hello"}}]}',
        'data: {"choices":[{"delta":{"content":" world"}}]}',
        'data: [DONE]',
    ]
    client = OpenAICompatibleLLM(base_url="http://fake", api_key="fake")

    # 替换requests.post
    import llm_client
    orig_post = llm_client.requests.post
    llm_client.requests.post = lambda *a, **kw: FakeSSEResponse(sse_lines)
    try:
      events = list(client.chat_stream([], []))
    finally:
      llm_client.requests.post = orig_post

    content_events = [e for e in events if e["type"] == "content"]
    done_events = [e for e in events if e["type"] == "done"]

    self.assertEqual(len(content_events), 2)
    self.assertEqual(content_events[0]["text"], "Hello")
    self.assertEqual(content_events[1]["text"], " world")
    self.assertEqual(len(done_events), 1)
    self.assertEqual(done_events[0]["message"]["content"], "Hello world")

  def test_sse_tool_calls_assembly(self):
    """解析SSE tool_calls增量片段并组装完整tool_calls"""
    sse_lines = [
        'data: {"choices":[{"delta":{"tool_calls":[{"index":0,"id":"call_1","function":{"name":"load_toolsets"}}]}}]}',
        'data: {"choices":[{"delta":{"tool_calls":[{"index":0,"function":{"arguments":"{\\"toolset_ids\\":"}}]}}]}',
        'data: {"choices":[{"delta":{"tool_calls":[{"index":0,"function":{"arguments":" [\\"ts_1\\"]}"}}]}}]}',
        'data: [DONE]',
    ]
    client = OpenAICompatibleLLM(base_url="http://fake", api_key="fake")

    import llm_client
    orig_post = llm_client.requests.post
    llm_client.requests.post = lambda *a, **kw: FakeSSEResponse(sse_lines)
    try:
      events = list(client.chat_stream([], []))
    finally:
      llm_client.requests.post = orig_post

    done_events = [e for e in events if e["type"] == "done"]
    self.assertEqual(len(done_events), 1)

    msg = done_events[0]["message"]
    self.assertIsNone(msg.get("content"))
    self.assertEqual(len(msg["tool_calls"]), 1)

    tc = msg["tool_calls"][0]
    self.assertEqual(tc["id"], "call_1")
    self.assertEqual(tc["function"]["name"], "load_toolsets")
    args = json.loads(tc["function"]["arguments"])
    self.assertEqual(args, {"toolset_ids": ["ts_1"]})

  def test_sse_done_marker(self):
    """data: [DONE]正确终止流"""
    sse_lines = [
        'data: {"choices":[{"delta":{"content":"OK"}}]}',
        'data: [DONE]',
        'data: {"choices":[{"delta":{"content":"should_not_appear"}}]}',
    ]
    client = OpenAICompatibleLLM(base_url="http://fake", api_key="fake")

    import llm_client
    orig_post = llm_client.requests.post
    llm_client.requests.post = lambda *a, **kw: FakeSSEResponse(sse_lines)
    try:
      events = list(client.chat_stream([], []))
    finally:
      llm_client.requests.post = orig_post

    content_events = [e for e in events if e["type"] == "content"]
    self.assertEqual(len(content_events), 1)
    self.assertEqual(content_events[0]["text"], "OK")

  def test_response_closed(self):
    """生成器关闭后response被close()"""
    sse_lines = ['data: [DONE]']
    client = OpenAICompatibleLLM(base_url="http://fake", api_key="fake")

    import llm_client
    orig_post = llm_client.requests.post
    fake_resp = FakeSSEResponse(sse_lines)
    llm_client.requests.post = lambda *a, **kw: fake_resp
    try:
      list(client.chat_stream([], []))
    finally:
      llm_client.requests.post = orig_post

    self.assertTrue(fake_resp._closed)

  def test_sse_reasoning_content_parsing(self):
    """解析SSE delta reasoning_content"""
    sse_lines = [
        'data: {"choices":[{"delta":{"reasoning_content":"思考中"}}]}',
        'data: {"choices":[{"delta":{"reasoning_content":"..."}}]}',
        'data: {"choices":[{"delta":{"content":"回复"}}]}',
        'data: [DONE]',
    ]
    client = OpenAICompatibleLLM(base_url="http://fake", api_key="fake")

    import llm_client
    orig_post = llm_client.requests.post
    llm_client.requests.post = lambda *a, **kw: FakeSSEResponse(sse_lines)
    try:
      events = list(client.chat_stream([], []))
    finally:
      llm_client.requests.post = orig_post

    reasoning_events = [e for e in events if e["type"] == "reasoning"]
    content_events = [e for e in events if e["type"] == "content"]
    done_events = [e for e in events if e["type"] == "done"]

    self.assertEqual(len(reasoning_events), 2)
    self.assertEqual(reasoning_events[0]["text"], "思考中")
    self.assertEqual(reasoning_events[1]["text"], "...")
    self.assertEqual(len(content_events), 1)
    self.assertEqual(content_events[0]["text"], "回复")
    self.assertEqual(len(done_events), 1)
    self.assertEqual(done_events[0]["message"]["content"], "回复")
    # reasoning_content不应出现在最终message中
    self.assertNotIn("reasoning_content", done_events[0]["message"])


if __name__ == "__main__":
  unittest.main()
