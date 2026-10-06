import json
import sys
import threading
import time

from config import AUDIO_ZONES, DEFAULT_USER_ID, DEFAULT_ZONE
from llm_config import EFFORT_MODES, PROVIDERS
from prefs import update_pref

# 导入 readline 让内置 input() 使用其行编辑器：终端原生行编辑按字节退格，
# 无法正确清除中文等宽字符，会在屏幕上残留半个字符。
try:
  import readline  # noqa: F401
  HAS_READLINE = True
except ImportError:
  HAS_READLINE = False

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
    self._thread: threading.Thread | None = None
    self._stop = threading.Event()
    self._old_settings: list | None = None
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
        kbhit = getattr(msvcrt, "kbhit", None)
        getch = getattr(msvcrt, "getch", None)
        while not self._stop.is_set():
          if kbhit is not None and kbhit():
            if getch is not None:
              ch = getch()
            else:
              ch = b""
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
        kbhit = getattr(msvcrt, "kbhit", None)
        getch = getattr(msvcrt, "getch", None)
        while kbhit is not None and kbhit():
          if getch is not None:
            getch()
    except Exception:
      pass


class CockpitRePL:
  """命令行交互式对话"""

  def __init__(self, agent):
    self.agent = agent
    self.running = False
    self.zone = DEFAULT_ZONE

  def run(self):
    self.running = True
    print("=" * 50)
    print("  智能座舱车载助手 - 交互模式")
    print("  命令: /exit 退出 | /clear 清空历史 | /history 查看历史 | /effort [none|low|high|max] 查看或切换思考深度 | /provider [id] 查看或切换 LLM 供应商 | /zone [front_left|front_right|rear_left|rear_right] 查看或切换音区")
    print("        /skill [name1 name2] 查看或加载技能 | /reload 重新发现技能并刷新上下文")
    print("  生成期间按 Esc 打断")
    print("=" * 50)

    while self.running:
      try:
        user_input = input(f"\n你[{self.zone}]: ").strip()
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
    parts = cmd.split(maxsplit=1)
    if parts[0] == "/exit":
      self.running = False
    elif parts[0] == "/clear":
      self._clear_history()
    elif parts[0] == "/history":
      self._show_history()
    elif parts[0] == "/effort":
      self._handle_effort(parts[1] if len(parts) > 1 else None)
    elif parts[0] == "/zone":
      self._handle_zone(parts[1] if len(parts) > 1 else None)
    elif parts[0] == "/provider":
      self._handle_provider(parts[1] if len(parts) > 1 else None)
    elif parts[0] == "/skill":
      self._handle_skill(parts[1].split() if len(parts) > 1 else [])
    elif parts[0] == "/reload":
      self._handle_reload()
    else:
      print(f"未知命令: {cmd}")
      print("可用命令: /exit /clear /history /effort /provider /zone /skill /reload")

  def _save_pref(self, key, value):
    """持久化偏好,失败仅警告,不影响内存切换"""
    try:
      update_pref(key, value)
    except OSError as e:
      print(f"[警告] 偏好持久化失败: {e}")

  def _handle_effort(self, mode_arg):
    if mode_arg is None:
      print(f"当前思考深度: {self.agent.effort}")
      print(f"可选值: {', '.join(EFFORT_MODES)}")
      return
    if mode_arg not in EFFORT_MODES:
      print(f"无效的思考深度: {mode_arg}")
      print(f"可选值: {', '.join(EFFORT_MODES)}")
      return
    self.agent.effort = mode_arg
    self._save_pref("effort", mode_arg)
    print(f"思考深度已切换为: {mode_arg}")

  def _handle_zone(self, zone_arg):
    if zone_arg is None:
      print(f"当前音区: {self.zone}")
      print(f"可选值: {', '.join(AUDIO_ZONES)}")
      return
    if zone_arg not in AUDIO_ZONES:
      print(f"无效的音区: {zone_arg}")
      print(f"可选值: {', '.join(AUDIO_ZONES)}")
      return
    self.zone = zone_arg
    print(f"音区已切换为: {zone_arg}")

  def _handle_provider(self, provider_arg):
    if provider_arg is None:
      print(f"当前供应商: {self.agent.provider_id}")
      print(f"可选值: {', '.join(PROVIDERS.keys())}")
      return
    try:
      self.agent.set_provider(provider_arg)
    except ValueError:
      print(f"无效的供应商: {provider_arg}")
      print(f"可选值: {', '.join(PROVIDERS.keys())}")
      return
    self._save_pref("provider", provider_arg)
    model = getattr(self.agent.llm, "model", "mock")
    print(f"供应商已切换为: {provider_arg} (模型: {model})")

  def _handle_skill(self, skill_args):
    skill_mgr = self.agent.skill_manager
    # 无参数：列出所有已安装技能
    if not skill_args:
      names = skill_mgr.get_all_skill_names()
      if not names:
        print(f"暂无已安装技能（技能目录: {skill_mgr.skills_dir}）")
        return
      print(f"已安装技能({len(names)}个, 目录: {skill_mgr.skills_dir}):")
      for n in names:
        print(f"  - {n}")
      return
    # 有参数：显式加载指定技能，模拟完整工具调用链（assistant tool_call + tool result）
    result = skill_mgr.load_skills(skill_args)
    call_id = f"call_skill_repl_{len(self.agent.messages)}"
    self.agent.messages.append({
        "role": "assistant",
        "content": None,
        "tool_calls": [{
            "id": call_id,
            "type": "function",
            "function": {
                "name": "load_skills",
                "arguments": json.dumps({"skill_names": skill_args}, ensure_ascii=False)
            }
        }]
    })
    self.agent.messages.append({
        "role": "tool",
        "tool_call_id": call_id,
        "content": json.dumps(result, ensure_ascii=False)
    })
    # 摘要展示（隐藏长 content 字段）
    summary = {
        k: (f"<{len(v)} chars>" if k == "content" else v)
        for k, v in result.items()
    }
    print(json.dumps(summary, ensure_ascii=False, indent=2))

  def _handle_reload(self):
    self.agent.reload_skills()
    names = self.agent.skill_manager.get_all_skill_names()
    print(f"技能已重新发现，当前可用技能({len(names)}个): {', '.join(names) or '无'}")

  def _clear_history(self):
    from config import build_system_prompt
    self.agent.messages = [
        {"role": "system",
         "content": build_system_prompt(
             self.agent.toolset_manager.get_toolset_listing(),
             self.agent.skill_manager.get_skill_listing())}
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

    cur_turn = 0
    cur_turn_start = 0.0
    cur_turn_prefilled = False
    cur_turn_prefill_time = 0.0
    cur_turn_content_started = False
    cur_turn_reasoning_started = False
    tool_start = 0.0      # 当前工具执行开始时间
    llm_time = 0.0       # LLM累计耗时
    tool_time = 0.0       # 工具累计耗时

    def on_call_llm(turn: int, start: bool):
      nonlocal cur_turn, cur_turn_start, cur_turn_prefilled, cur_turn_prefill_time
      nonlocal llm_time, cur_turn_content_started, cur_turn_reasoning_started
      if start:
        cur_turn_start = time.time()
        cur_turn = turn
        cur_turn_prefilled = False
        cur_turn_prefill_time = 0.0
        cur_turn_reasoning_started = False
        cur_turn_content_started = False
        print(f"\n\n第{cur_turn}轮llm推理开始...")
      else:
        cur_turn_end = time.time()
        cur_turn_llm_time = cur_turn_end - cur_turn_start
        llm_time += cur_turn_llm_time
        print(f"\n\n第{turn}轮llm推理结束, prefill耗时:{cur_turn_prefill_time - cur_turn_start:.2f}s, 总耗时:{cur_turn_llm_time:.2f}s")

    def on_reasoning(text: str):
      nonlocal cur_turn_prefilled, cur_turn_prefill_time, cur_turn_reasoning_started, cur_turn
      if not cur_turn_prefilled:
        cur_turn_prefilled = True
        cur_turn_prefill_time = time.time()
      if not cur_turn_reasoning_started:
        cur_turn_reasoning_started = True
        print(f"\n\n[第{cur_turn}轮思考中] ", end="", flush=True)
      print(text, end="", flush=True)

    def on_content(text: str):
      nonlocal cur_turn_prefilled, cur_turn_prefill_time, cur_turn_content_started, cur_turn
      if not cur_turn_prefilled:
        cur_turn_prefilled = True
        cur_turn_prefill_time = time.time()
      if not cur_turn_content_started:
        cur_turn_content_started = True
        print(f"\n\n[第{cur_turn}轮回复中] ", end="", flush=True)
      print(text, end="", flush=True)

    def on_tool_call(name: str, args: dict):
      nonlocal tool_start, cur_turn
      tool_start = time.time()
      print(f"\n\n[第{cur_turn}轮工具调用] {name}({json.dumps(args, ensure_ascii=False)})")

    def on_tool_result(name: str, result: dict):
      nonlocal tool_start, tool_time, cur_turn
      elapsed = time.time() - tool_start
      tool_time += elapsed
      print(f"[第{cur_turn}轮工具结果] {name}: {json.dumps(result, ensure_ascii=False)} (耗时: {elapsed:.2f}s)")

    try:
      self.agent.chat_stream(
          user_input,
          on_reasoning=on_reasoning,
          on_content=on_content,
          on_tool_call=on_tool_call,
          on_tool_result=on_tool_result,
          on_call_llm=on_call_llm,
          zone_id=self.zone,
          user_id=DEFAULT_USER_ID
      )
      total = time.time() - task_start
      print(f"\n\n[耗时(共{cur_turn}轮)] LLM: {llm_time:.2f}s | 工具: {tool_time:.2f}s | 总计: {total:.2f}s")
    except InterruptedError:
      total = time.time() - task_start
      print(f"\n\n[已打断] (耗时: {total:.2f}s)")
    finally:
      listener.stop()
