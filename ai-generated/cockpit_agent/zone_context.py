import re

from config import DEFAULT_USER_ID, DEFAULT_ZONE


_USER_TAG_RE = re.compile(r"^\[zone=(?P<zone>[^\],]+),user=(?P<user>[^\]]+)\]\s?(?P<text>.*)$", re.DOTALL)


def tag_user_message(text: str, zone_id: str = DEFAULT_ZONE, user_id: str = DEFAULT_USER_ID) -> str:
  """把用户输入包装成模型可见的多音区消息。"""
  return f"[zone={zone_id},user={user_id}] {text}"


def parse_user_tag(content: str) -> tuple[str | None, str | None, str]:
  """解析 [zone=xxx,user=yyy] 前缀，返回(音区,用户,正文)。"""
  match = _USER_TAG_RE.match(content)
  if not match:
    return None, None, content
  return match.group("zone"), match.group("user"), match.group("text")
