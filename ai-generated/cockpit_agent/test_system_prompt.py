"""System Prompt 模板单元测试"""
import unittest

from config import SYSTEM_PROMPT_TEMPLATE, build_system_prompt


class SystemPromptTest(unittest.TestCase):

  def test_contains_language_requirement(self):
    """SP 必须包含语言要求段落并明确强调简体中文"""
    self.assertIn("## 语言要求", SYSTEM_PROMPT_TEMPLATE)
    self.assertIn("禁止使用英文回复", SYSTEM_PROMPT_TEMPLATE)

  def test_reiterates_language_at_end(self):
    """末尾回复风格再次强调简体中文,形成前后呼应"""
    self.assertIn("## 回复风格", SYSTEM_PROMPT_TEMPLATE)
    self.assertIn("简体中文", SYSTEM_PROMPT_TEMPLATE.rsplit("## 全量工具集", 1)[-1])

  def test_prohibits_reading_tech_identifiers(self):
    """明确禁止在回复中朗读工具名/参数名/JSON"""
    self.assertIn("禁止出现工具名", SYSTEM_PROMPT_TEMPLATE)

  def test_template_fills_toolset_listing(self):
    """模板仅有 toolset_listing 占位符且可正确替换"""
    sp = build_system_prompt("- toolset_demo:演示工具集,仅用于测试")
    self.assertIn("- toolset_demo:演示工具集,仅用于测试", sp)
    self.assertNotIn("{toolset_listing}", sp)


if __name__ == "__main__":
  unittest.main()
