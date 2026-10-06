"""System Prompt 模板单元测试"""
import unittest

from config import SYSTEM_PROMPT_TEMPLATE, build_system_prompt


class SystemPromptTest(unittest.TestCase):

  def test_contains_language_requirement(self):
    """SP 包含禁止英文回复的语言要求"""
    self.assertIn("禁止使用英文回复", SYSTEM_PROMPT_TEMPLATE)

  def test_defines_role_in_chinese(self):
    """角色定义段强调使用简体中文"""
    self.assertIn("## 角色定义", SYSTEM_PROMPT_TEMPLATE)
    self.assertIn("使用简体中文", SYSTEM_PROMPT_TEMPLATE)

  def test_prohibits_reading_tech_identifiers(self):
    """明确禁止在回复中朗读工具集名/工具名/参数名/JSON"""
    self.assertIn("禁止回复出现工具集名、工具名、参数名", SYSTEM_PROMPT_TEMPLATE)

  def test_template_fills_toolset_and_skill_listing(self):
    """模板的 toolset_listing 与 skill_listing 占位符均可正确替换"""
    sp = build_system_prompt(
        "- toolset_demo:演示工具集,仅用于测试",
        "- skill_demo:演示技能,仅用于测试",
    )
    self.assertIn("- toolset_demo:演示工具集,仅用于测试", sp)
    self.assertIn("- skill_demo:演示技能,仅用于测试", sp)
    self.assertNotIn("{toolset_listing}", sp)
    self.assertNotIn("{skill_listing}", sp)

  def test_contains_skill_section(self):
    """SP 包含技能使用指引：通过 load_skills 加载、内容注入上下文、无数量限制"""
    self.assertIn("## 已安装技能", SYSTEM_PROMPT_TEMPLATE)
    self.assertIn("load_skills", SYSTEM_PROMPT_TEMPLATE)
    self.assertIn("注入当前对话上下文", SYSTEM_PROMPT_TEMPLATE)
    self.assertIn("技能无数量限制", SYSTEM_PROMPT_TEMPLATE)


if __name__ == "__main__":
  unittest.main()
