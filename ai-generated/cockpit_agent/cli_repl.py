import json
import sys
import threading
import time

try:
  import termios
  import tty
  HAS_TERMIOS = True
except ImportError:
  HAS_TERMIOS = False

try:
  import msvcrt
  HAS_MSVCRT = True
except ImportError:
  HAS_MSVCRT = False


class EscListener:
  """在生成期间监听Esc键，检测到后触发回调"""

  def __init__(self, on_esc):
    self.on_esc = on_esc
    self._thread = None
    self._stop = threading.Event()
    self._old_settings = None
    self._is_tty = hasattr(sys.stdin, "isatty") and sys.stdin.isatty()

  def start(self):
    if not self._is_tty or not (HAS_TERMIOS or HAS_MSVCRT):
      return
    self._stop.clear()
    self._thread = threading.Thread(target=self._run, daemon=True)
    self._thread.start()

  def stop(self):
    self._stop.set()
    if self._thread:
      self._thread.join(timeout=1.0)
      self._thread = None
    self._restore()
    self._flush_input()

  def _run(self):
    try:
      if HAS_TERMIOS:
        import select
        fd = sys.stdin.fileno()
        self._old_settings = termios.tcgetattr(fd)
        tty.setcbreak(fd)
        while not self._stop.is_set():
          r, _, _ = select.select([sys.stdin], [], [], 0.05)
          if r:
            ch = sys.stdin.read(1)
            if ch == "\x1b":
              self.on_esc()
              return
      elif HAS_MSVCRT:
        while not self._stop.is_set():
          if msvcrt.kbhit():
            ch = msvcrt.getch()
            if ch == b"\x1b":
              self.on_esc()
              return
          time.sleep(0.01)
    except Exception:
      pass
    finally:
      self._restore()

  def _restore(self):
    if self._old_settings and HAS_TERMIOS:
      try:
        termios.tcsetattr(sys.stdin.fileno(), termios.TCSADRAIN, self._old_settings)
      except Exception:
        pass
      self._old_settings = None

  def _flush_input(self):
    """清空生成期间残留的输入，避免污染下一次input()"""
    if not self._is_tty:
      return
    try:
      if HAS_TERMIOS:
        termios.tcflush(sys.stdin.fileno(), termios.TCIFLUSH)
      elif HAS_MSVCRT:
        while msvcrt.kbhit():
          msvcrt.getch()
    except Exception:
      pass


class CockpitRePL:
  """命令行交互式对话"""

  def __init__(self, agent):
    self.agent = agent
    self.running = False

  def run(self):
    self.running = True
    print("=" * 50)
    print("  智能座舱车载助手 - 交互模式")
    print("  命令: /exit 退出 | /clear 清空历史 | /history 查看历史")
    print("  生成期间按 Esc 打断")
    print("=" * 50)

    while self.running:
      try:
        user_input = input("\n你: ").strip()
      except EOFError:
        break
      except KeyboardInterrupt:
        print()
        break

      if not user_input:
        continue

      if user_input.startswith("/"):
        self._handle_command(user_input)
        continue

      self._chat(user_input)

    print("\n再见！")

  def _handle_command(self, cmd):
    if cmd == "/exit":
      self.running = False
    elif cmd == "/clear":
      self._clear_history()
    elif cmd == "/history":
      self._show_history()
    else:
      print(f"未知命令: {cmd}")
      print("可用命令: /exit /clear /history")

  def _clear_history(self):
    from config import build_system_prompt
    self.agent.messages = [
        {"role": "system",
         "content": build_system_prompt(self.agent.toolset_manager.get_toolset_listing())}
    ]
    print("对话历史已清空。")

  def _show_history(self):
    if len(self.agent.messages) <= 1:
      print("暂无对话历史。")
      return
    for msg in self.agent.messages[1:]:
      role = msg["role"]
      content = str(msg.get("content", ""))
      if len(content) > 60:
        content = content[:60] + "..."
      print(f"  [{role}] {content}")

  def _chat(self, user_input):
    listener = EscListener(on_esc=self.agent.interrupt)
    listener.start()

    task_start = time.time()
    # current: None / "reasoning" / "reply"
    # start: 当前流式区域开始时间
    # tool_start: 当前工具执行开始时间
    section = {"current": None, "start": None, "tool_start": None}
    stats = {"llm": 0.0, "tool": 0.0}

    def _close_streaming():
      if section["current"] is not None:
        elapsed = time.time() - section["start"]
        cumulative = time.time() - task_start
        stats["llm"] += elapsed
        print(f" (耗时: {elapsed:.2f}s, 累积: {cumulative:.2f}s)")
        section["current"] = None
        section["start"] = None

    def on_reasoning(text):
      if section["current"] != "reasoning":
        _close_streaming()
        section["start"] = time.time()
        print("[思考中] ", end="", flush=True)
        section["current"] = "reasoning"
      print(text, end="", flush=True)

    def on_content(text):
      if section["current"] != "reply":
        _close_streaming()
        section["start"] = time.time()
        print("[回复中] ", end="", flush=True)
        section["current"] = "reply"
      print(text, end="", flush=True)

    def on_tool_call(name, args):
      _close_streaming()
      section["tool_start"] = time.time()
      print(f"[工具调用] {name}({json.dumps(args, ensure_ascii=False)})")

    def on_tool_result(name, result):
      elapsed = time.time() - section["tool_start"]
      cumulative = time.time() - task_start
      stats["tool"] += elapsed
      print(f"[工具结果] {name}: {json.dumps(result, ensure_ascii=False)} (耗时: {elapsed:.2f}s, 累积: {cumulative:.2f}s)")
      section["tool_start"] = None

    try:
      reply = self.agent.chat_stream(
          user_input,
          on_reasoning=on_reasoning,
          on_content=on_content,
          on_tool_call=on_tool_call,
          on_tool_result=on_tool_result
      )
      _close_streaming()
      total = time.time() - task_start
      print(f"[耗时] LLM: {stats['llm']:.2f}s | 工具: {stats['tool']:.2f}s | 总计: {total:.2f}s")
    except InterruptedError:
      _close_streaming()
      total = time.time() - task_start
      print(f"[已打断] (耗时: {total:.2f}s)")
    finally:
      section["current"] = None
      listener.stop()
