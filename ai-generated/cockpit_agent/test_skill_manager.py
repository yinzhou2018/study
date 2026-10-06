"""技能管理器单元测试"""
import shutil
import tempfile
import unittest
from pathlib import Path

from skill_manager import SkillManager, _load_skill_file, _parse_frontmatter


class FrontmatterParserTest(unittest.TestCase):

  def test_parse_frontmatter_with_yaml(self):
    content = (
        "---\n"
        "name: test-skill\n"
        "description: A test skill for unit testing.\n"
        "---\n"
        "# Test Skill\n\n"
        "Skill content goes here."
    )
    fm, body = _parse_frontmatter(content)
    self.assertEqual(fm["name"], "test-skill")
    self.assertEqual(fm["description"], "A test skill for unit testing.")
    self.assertIn("Skill content goes here.", body)

  def test_parse_frontmatter_missing_description(self):
    content = (
        "---\n"
        "name: no-desc\n"
        "---\n"
        "# No description\n"
        "Body."
    )
    fm, body = _parse_frontmatter(content)
    self.assertEqual(fm["name"], "no-desc")
    self.assertNotIn("description", fm)

  def test_parse_frontmatter_no_frontmatter(self):
    content = (
        "# Just a heading\n"
        "\n"
        "Some content."
    )
    fm, body = _parse_frontmatter(content)
    self.assertEqual(fm, {})
    self.assertEqual(body, content)

  def test_parse_frontmatter_quoted_values(self):
    content = (
        "---\n"
        'name: "quoted-name"\n'
        "description: 'a quoted description'\n"
        "---\n"
        "body"
    )
    fm, body = _parse_frontmatter(content)
    self.assertEqual(fm["name"], "quoted-name")
    self.assertEqual(fm["description"], "a quoted description")


class LoadSkillFileTest(unittest.TestCase):

  def setUp(self):
    self.tmp = tempfile.mkdtemp()
    self.skill_dir = Path(self.tmp) / "my-skill"
    self.skill_dir.mkdir()

  def tearDown(self):
    shutil.rmtree(self.tmp)

  def test_load_valid_skill(self):
    (self.skill_dir / "SKILL.md").write_text(
        "---\n"
        "name: my-skill\n"
        "description: Does something useful.\n"
        "---\n"
        "# My Skill\n\n"
        "Use this when you need it.",
        encoding="utf-8"
    )
    skill = _load_skill_file(self.skill_dir)
    self.assertIsNotNone(skill)
    self.assertEqual(skill["name"], "my-skill")
    self.assertEqual(skill["description"], "Does something useful.")
    self.assertIn("Use this when you need it.", skill["content"])

  def test_load_skill_missing_description_returns_none(self):
    (self.skill_dir / "SKILL.md").write_text(
        "---\n"
        "name: no-desc\n"
        "---\n"
        "body",
        encoding="utf-8"
    )
    self.assertIsNone(_load_skill_file(self.skill_dir))

  def test_missing_skills_md_returns_none(self):
    self.assertIsNone(_load_skill_file(self.skill_dir))


class SkillManagerTest(unittest.TestCase):

  def setUp(self):
    self.tmp = tempfile.mkdtemp()
    self.skills_dir = Path(self.tmp)

    # 创建测试技能1
    self.s1 = self.skills_dir / "skill-one"
    self.s1.mkdir()
    (self.s1 / "SKILL.md").write_text(
        "---\n"
        "name: skill-one\n"
        "description: First skill for testing.\n"
        "---\n"
        "# Skill One\n\n"
        "One content.",
        encoding="utf-8"
    )

    # 创建测试技能2
    self.s2 = self.skills_dir / "skill-two"
    self.s2.mkdir()
    (self.s2 / "SKILL.md").write_text(
        "---\n"
        "name: skill-two\n"
        "description: Second skill for testing.\n"
        "---\n"
        "# Skill Two\n\n"
        "Two content.",
        encoding="utf-8"
    )

    self.mgr = SkillManager(skills_dir=self.skills_dir)

  def tearDown(self):
    shutil.rmtree(self.tmp)

  def test_discover_skills(self):
    names = self.mgr.get_all_skill_names()
    self.assertEqual(sorted(names), ["skill-one", "skill-two"])

  def test_is_valid_skill(self):
    self.assertTrue(self.mgr.is_valid_skill("skill-one"))
    self.assertTrue(self.mgr.is_valid_skill("skill-two"))
    self.assertFalse(self.mgr.is_valid_skill("nonexistent"))

  def test_load_skills_success(self):
    result = self.mgr.load_skills(["skill-one"])
    self.assertEqual(result["status"], "success")
    self.assertEqual(len(result["loaded"]), 1)
    self.assertEqual(result["loaded"][0]["skill_name"], "skill-one")
    self.assertEqual(len(result["failed"]), 0)
    self.assertEqual(len(result["injected"]), 1)
    self.assertEqual(result["injected"][0]["role"], "system")
    self.assertIn("【技能:skill-one】", result["injected"][0]["content"])
    self.assertIn("One content", result["injected"][0]["content"])

  def test_load_skills_partial_failure(self):
    result = self.mgr.load_skills(["skill-one", "nonexistent", "skill-two"])
    self.assertEqual(result["status"], "partial_success")
    self.assertEqual(len(result["loaded"]), 2)
    self.assertEqual(len(result["failed"]), 1)
    self.assertEqual(result["failed"][0]["skill_name"], "nonexistent")
    self.assertEqual(len(result["injected"]), 2)

  def test_load_skills_all_failure(self):
    result = self.mgr.load_skills(["ghost"])
    self.assertEqual(result["status"], "partial_success")  # 与 load_toolsets 保持一致
    self.assertEqual(len(result["loaded"]), 0)
    self.assertEqual(result["failed"][0]["reason"], "技能不存在")

  def test_load_skills_idempotent(self):
    """同一技能重复加载应成功（无 LRU 淘汰，无数量限制）"""
    r1 = self.mgr.load_skills(["skill-one"])
    r2 = self.mgr.load_skills(["skill-one"])
    self.assertEqual(r1["status"], "success")
    self.assertEqual(r2["status"], "success")

  def test_get_skill_listing(self):
    listing = self.mgr.get_skill_listing()
    self.assertIn("skill-one:First skill for testing.", listing)
    self.assertIn("skill-two:Second skill for testing.", listing)

  def test_get_system_tool(self):
    tool = self.mgr.get_system_tool(["skill-one", "skill-two"])
    self.assertEqual(tool["function"]["name"], "load_skills")
    enum = tool["function"]["parameters"]["properties"]["skill_names"]["items"]["enum"]
    self.assertEqual(sorted(enum), ["skill-one", "skill-two"])

  def test_empty_skills_dir(self):
    empty_dir = Path(tempfile.mkdtemp())
    try:
      mgr = SkillManager(skills_dir=empty_dir)
      self.assertEqual(mgr.get_all_skill_names(), [])
      self.assertEqual(mgr.get_skill_listing(), "（暂无已安装的技能）")
    finally:
      shutil.rmtree(empty_dir)

  def test_discover_refreshes_after_change(self):
    """技能目录变化后 discover_skills 能感知增删"""
    s3 = self.skills_dir / "skill-three"
    s3.mkdir()
    (s3 / "SKILL.md").write_text(
        "---\n"
        "name: skill-three\n"
        "description: Third skill.\n"
        "---\n"
        "Three content.",
        encoding="utf-8"
    )
    self.mgr.discover_skills()
    self.assertIn("skill-three", self.mgr.get_all_skill_names())

    # 删除后重新发现
    shutil.rmtree(self.s2)
    self.mgr.discover_skills()
    self.assertNotIn("skill-two", self.mgr.get_all_skill_names())


class SkillAgentIntegrationTest(unittest.TestCase):
  """SkillManager 与 CockpitAgent 集成（临时目录隔离，不依赖真实用户目录）"""

  def _make_skill(self, root, name, desc):
    d = root / name
    d.mkdir()
    (d / "SKILL.md").write_text(
        "---\n"
        f"name: {name}\n"
        f"description: {desc}\n"
        "---\n"
        f"# {name}\n"
        f"Instruction for {name}.",
        encoding="utf-8"
    )

  def test_system_prompt_contains_skill_listing(self):
    from cockpit_agent import CockpitAgent
    from llm_client import MockLLMClient

    tmp = Path(tempfile.mkdtemp())
    try:
      self._make_skill(tmp, "alpha", "Alpha skill.")
      agent = CockpitAgent(llm_client=MockLLMClient(), skills_dir=tmp)
      sp = agent.messages[0]["content"]
      self.assertIn("alpha:Alpha skill.", sp)
      self.assertIn("load_skills", sp)
    finally:
      shutil.rmtree(tmp)

  def test_current_tools_include_load_skills(self):
    from cockpit_agent import CockpitAgent
    from llm_client import MockLLMClient

    tmp = Path(tempfile.mkdtemp())
    try:
      self._make_skill(tmp, "alpha", "Alpha skill.")
      agent = CockpitAgent(llm_client=MockLLMClient(), skills_dir=tmp)
      tool_names = [t["function"]["name"] for t in agent._get_current_tools()]
      self.assertIn("load_skills", tool_names)
    finally:
      shutil.rmtree(tmp)

  def test_reload_skills_refreshes_sp(self):
    from cockpit_agent import CockpitAgent
    from llm_client import MockLLMClient

    tmp = Path(tempfile.mkdtemp())
    try:
      self._make_skill(tmp, "alpha", "Alpha skill.")
      agent = CockpitAgent(llm_client=MockLLMClient(), skills_dir=tmp)
      self.assertNotIn("beta:Beta skill.", agent.messages[0]["content"])

      # 运行期新增技能并 /reload
      self._make_skill(tmp, "beta", "Beta skill.")
      agent.reload_skills()
      self.assertIn("beta:Beta skill.", agent.messages[0]["content"])
    finally:
      shutil.rmtree(tmp)

  def test_chat_load_skills_injects_context(self):
    """模型调用 load_skills 后，技能内容作为 system 消息注入对话上下文"""
    from cockpit_agent import CockpitAgent

    class FakeSkillLLM:
      """第一轮返回 load_skills 工具调用，第二轮返回最终文本"""

      def __init__(self):
        self.calls = 0

      def chat(self, messages, tools, temperature=0.1, effort=None):
        self.calls += 1
        if self.calls == 1:
          return {
              "choices": [{
                  "message": {
                      "role": "assistant",
                      "content": None,
                      "tool_calls": [{
                          "id": "call_skill_1",
                          "type": "function",
                          "function": {
                              "name": "load_skills",
                              "arguments": '{"skill_names": ["alpha"]}'
                          }
                      }]
                  },
                  "finish_reason": "tool_calls"
              }]
          }
        return {
            "choices": [{
                "message": {"role": "assistant", "content": "技能已生效。"},
                "finish_reason": "stop"
            }]
        }

    tmp = Path(tempfile.mkdtemp())
    try:
      self._make_skill(tmp, "alpha", "Alpha skill.")
      llm = FakeSkillLLM()
      agent = CockpitAgent(llm_client=llm, skills_dir=tmp)
      reply = agent.chat("加载技能", verbose=False)
      self.assertEqual(reply, "技能已生效。")
      # 技能内容已注入上下文
      injected = [m for m in agent.messages
                  if m["role"] == "system" and "【技能:alpha】" in m["content"]]
      self.assertEqual(len(injected), 1)
      self.assertIn("Instruction for alpha.", injected[0]["content"])
      # 工具结果消息也已回填
      tool_msgs = [m for m in agent.messages if m["role"] == "tool"]
      self.assertEqual(len(tool_msgs), 1)
      self.assertIn("成功加载1个技能", tool_msgs[0]["content"])
    finally:
      shutil.rmtree(tmp)


if __name__ == "__main__":
  unittest.main()
