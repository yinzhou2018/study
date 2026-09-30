"""provider/effort 等用户偏好的本地持久化存储。

持久化文件位于用户目录 ~/.cockpit_agent/prefs.json,不进版本库。
本模块只负责纯存取与优先级解析,不依赖业务配置(PROVIDERS/EFFORT_MODES),
校验规则(valid 集合)由调用方传入,避免循环依赖。
"""
import json
import os

PREFS_DIR = os.path.expanduser("~/.cockpit_agent")
PREFS_FILE = os.path.join(PREFS_DIR, "prefs.json")


def load_prefs():
  """读取本地偏好,文件不存在或损坏时返回空dict"""
  try:
    with open(PREFS_FILE, "r", encoding="utf-8") as f:
      data = json.load(f)
  except (FileNotFoundError, json.JSONDecodeError, OSError):
    return {}
  return data if isinstance(data, dict) else {}


def save_prefs(prefs):
  """原子写:先写临时文件再rename,避免写一半损坏"""
  os.makedirs(PREFS_DIR, exist_ok=True)
  tmp = PREFS_FILE + ".tmp"
  with open(tmp, "w", encoding="utf-8") as f:
    json.dump(prefs, f, ensure_ascii=False, indent=2)
  os.replace(tmp, PREFS_FILE)


def update_pref(key, value):
  """更新单个偏好项:读-改-写"""
  prefs = load_prefs()
  prefs[key] = value
  save_prefs(prefs)


def resolve_pref(cli_val, prefs, key, valid, default):
  """优先级:命令行显式参数 > 持久化偏好 > 代码默认值;非法持久化值回退默认"""
  if cli_val:
    return cli_val
  pref = prefs.get(key)
  return pref if pref in valid else default
