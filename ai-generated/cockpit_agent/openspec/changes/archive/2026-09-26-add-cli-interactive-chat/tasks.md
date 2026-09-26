## 1. LLM Streaming Support

- [x] 1.1 Add `chat_stream` generator method to `OpenAICompatibleLLM` using `requests.post(stream=True)` and SSE line parsing, yielding delta content and accumulating tool_calls fragments
- [x] 1.2 Add `chat_stream` generator method to `MockLLMClient` that reuses existing `chat()` decision logic and yields tokens with small delays to simulate streaming
- [x] 1.3 Add unit tests for SSE parsing, delta content accumulation, and tool_calls fragment assembly

## 2. Agent Streaming Refactor

- [x] 2.1 Add `CockpitAgent.chat_stream(user_query, on_token, on_tool_call, on_tool_result, on_done)` callback-driven method that reuses the existing tool-call loop and `load_toolsets` special handling
- [x] 2.2 Keep `CockpitAgent.chat()` signature and behavior unchanged by collecting streamed tokens internally and returning the final reply string
- [x] 2.3 Add interrupt flag checking (`self._interrupted`) before each token yield and each tool execution, raising `InterruptedError` when set
- [x] 2.4 Add unit tests for streaming callbacks, tool-call event emission, and backward-compatible `chat()` behavior

## 3. CLI REPL Module

- [x] 3.1 Create `cli_repl.py` with a `CockpitRePL` class managing the input loop, prompt display, and command dispatch
- [x] 3.2 Implement slash commands: `/exit` (terminate), `/clear` (reset history to system prompt), `/history` (list messages with role and truncated preview), and unknown-command error listing available commands
- [x] 3.3 Implement streaming renderers: `[思考中]` for reasoning tokens, `[最终回复]` for final reply tokens, `[工具调用]` for tool name plus arguments, `[工具结果]` for tool result summary
- [x] 3.4 Register Esc key listener using `tty.setraw()` + `select` on stdin during generation, setting the interrupt flag when Esc (0x1B) is detected; use `msvcrt` on Windows as fallback
- [x] 3.5 On interrupt, print `[已打断]`, preserve completed tool results in history, and return to the prompt

## 4. Entry Point Integration

- [x] 4.1 Add `--interactive` / `-i` argument parsing to `main.py`; default remains batch demo mode
- [x] 4.2 Wire the interactive flag to instantiate `CockpitRePL` with the configured LLM client and start the loop
- [x] 4.3 Verify `python main.py` still runs the existing batch demo unchanged

## 5. Verification

- [x] 5.1 Run all unit tests covering streaming, callbacks, commands, and interrupt behavior
- [x] 5.2 Manually verify interactive mode with `MockLLMClient`: multi-turn dialogue, tool call display, `/history`, `/clear`, `/exit`, and Esc key interrupt
- [x] 5.3 Verify real LLM streaming against the OpenAI-compatible endpoint if credentials are available; otherwise document the limitation
  - Note: Sandbox environment blocks network access to the real endpoint. SSE parsing is covered by unit tests with mocked SSE responses (`test_llm_stream.py`). Real endpoint verification requires running outside the sandbox.
