"""
技能管理器：负责从用户目录发现技能、解析 SKILL.md、注入上下文。

发现路径：~/.cockpit_agent/skills/
遵循 Agent Skills 规范（https://agentskills.io/specification），支持 frontmatter + 扩展字段。
"""
import re
from pathlib import Path
from typing import Optional

DEFAULT_SKILLS_DIR = Path.home() / ".cockpit_agent" / "skills"


def _parse_frontmatter(content: str) -> tuple[dict, str]:
  """解析 Markdown 文件的 YAML frontmatter，返回 (frontmatter_dict, body_content)"""
  fm_match = re.match(r"^---\n(.*?)\n---\n(.*)$", content, re.DOTALL)
  if not fm_match:
    return {}, content

  fm_lines = fm_match.group(1).strip().splitlines()
  body = fm_match.group(2).strip()

  fm = {}
  for line in fm_lines:
    if ":" not in line:
      continue
    key, _, val = line.partition(":")
    val = val.strip().strip('"').strip("'")
    fm[key.strip()] = val

  return fm, body


def _load_skill_file(skill_dir: Path) -> Optional[dict]:
  """加载单个技能目录下的 SKILL.md，返回结构化数据"""
  skill_md = skill_dir / "SKILL.md"
  if not skill_md.is_file():
    return None

  try:
    content = skill_md.read_text(encoding="utf-8")
  except Exception:
    return None

  fm, body = _parse_frontmatter(content)
  name = fm.get("name") or skill_dir.name
  description = fm.get("description")

  if not description:
    return None  # 规范要求必须有 description

  return {
      "name": name,
      "description": description,
      "skill_dir": str(skill_dir),
      "content": body,
      "frontmatter": fm,
  }


class SkillManager:
  def __init__(self, skills_dir: Path | str | None = None):
    self.skills_dir = Path(skills_dir) if skills_dir else DEFAULT_SKILLS_DIR
    self._skills: dict[str, dict] = {}
    self.discover_skills()

  def discover_skills(self):
    """扫描 skills_dir 下所有含 SKILL.md 的子目录，构建技能索引"""
    self._skills.clear()
    if not self.skills_dir.is_dir():
      return

    for entry in self.skills_dir.iterdir():
      if not entry.is_dir():
        continue
      skill = _load_skill_file(entry)
      if skill:
        self._skills[skill["name"]] = skill

  def is_valid_skill(self, skill_name: str) -> bool:
    return skill_name in self._skills

  def load_skills(self, skill_names: list[str]) -> dict:
    """加载指定技能，返回结果（含待注入上下文消息列表 injected）"""
    loaded = []
    failed = []
    injected = []

    for name in skill_names:
      if name not in self._skills:
        failed.append({"skill_name": name, "reason": "技能不存在"})
        continue

      skill = self._skills[name]
      injected.append({
          "role": "system",
          "content": f"【技能:{skill['name']}】\n{skill['content']}"
      })
      loaded.append({
          "skill_name": skill["name"],
          "description": skill["description"],
      })

    status = "success" if not failed else "partial_success"
    return {
        "status": status,
        "loaded": loaded,
        "failed": failed,
        "injected": injected,
        "message": f"成功加载{len(loaded)}个技能，失败{len(failed)}个"
    }

  def get_skill_listing(self) -> str:
    """生成技能清单描述，供 System Prompt 使用"""
    if not self._skills:
      return "（暂无已安装的技能）"
    lines = []
    for name, skill in self._skills.items():
      desc = skill["description"]
      lines.append(f"- {name}:{desc}")
    return "\n".join(lines)

  def get_system_tool(self, skill_names: list[str]) -> dict:
    """构建 load_skills 系统工具，enum 为当前已发现的技能名"""
    return {
        "type": "function",
        "function": {
            "name": "load_skills",
            "description": "加载车载技能，加载后技能包含的操作指令即生效（若技能指引需要工具集，需再调用 load_toolsets）。技能无数量限制，可随时加载多个。",
            "parameters": {
                "type": "object",
                "properties": {
                    "skill_names": {
                        "type": "array",
                        "items": {
                            "type": "string",
                            "enum": skill_names
                        },
                        "minItems": 1,
                        "description": "要加载的技能名列表"
                    }
                },
                "required": ["skill_names"]
            }
        }
    }

  def get_all_skill_names(self) -> list[str]:
    return list(self._skills.keys())