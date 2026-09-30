"""prefs 本地偏好持久化与启动优先级解析单元测试"""
import json
import os
import shutil
import tempfile
import unittest

import prefs


class PrefsIOTest(unittest.TestCase):

  def setUp(self):
    self._tmpdir = tempfile.mkdtemp()
    self._orig_dir = prefs.PREFS_DIR
    self._orig_file = prefs.PREFS_FILE
    prefs.PREFS_DIR = os.path.join(self._tmpdir, "cockpit")
    prefs.PREFS_FILE = os.path.join(prefs.PREFS_DIR, "prefs.json")

  def tearDown(self):
    prefs.PREFS_DIR = self._orig_dir
    prefs.PREFS_FILE = self._orig_file
    shutil.rmtree(self._tmpdir, ignore_errors=True)

  def test_load_empty_when_no_file(self):
    self.assertEqual(prefs.load_prefs(), {})

  def test_save_then_load(self):
    prefs.save_prefs({"provider": "deepseek", "effort": "low"})
    self.assertEqual(prefs.load_prefs(),
                     {"provider": "deepseek", "effort": "low"})

  def test_update_pref_merges(self):
    prefs.save_prefs({"provider": "voyah"})
    prefs.update_pref("effort", "max")
    self.assertEqual(prefs.load_prefs(),
                     {"provider": "voyah", "effort": "max"})

  def test_load_corrupt_file_returns_empty(self):
    os.makedirs(prefs.PREFS_DIR)
    with open(prefs.PREFS_FILE, "w") as f:
      f.write("{not valid json")
    self.assertEqual(prefs.load_prefs(), {})

  def test_load_non_dict_returns_empty(self):
    os.makedirs(prefs.PREFS_DIR)
    with open(prefs.PREFS_FILE, "w") as f:
      json.dump(["a", "b"], f)
    self.assertEqual(prefs.load_prefs(), {})

  def test_save_creates_dir(self):
    self.assertFalse(os.path.exists(prefs.PREFS_DIR))
    prefs.save_prefs({"effort": "high"})
    self.assertTrue(os.path.exists(prefs.PREFS_FILE))

  def test_save_is_atomic_no_tmp_left(self):
    prefs.save_prefs({"effort": "high"})
    self.assertFalse(os.path.exists(prefs.PREFS_FILE + ".tmp"))


class ResolvePrefTest(unittest.TestCase):
  """优先级:命令行显式参数 > 持久化偏好 > 代码默认值"""

  VALID = {"none", "low", "high", "max"}

  def test_cli_arg_wins_over_persist(self):
    self.assertEqual(
        prefs.resolve_pref("low", {"effort": "high"}, "effort", self.VALID, "high"),
        "low")

  def test_persist_wins_over_default(self):
    self.assertEqual(
        prefs.resolve_pref(None, {"effort": "low"}, "effort", self.VALID, "high"),
        "low")

  def test_default_when_no_persist(self):
    self.assertEqual(
        prefs.resolve_pref(None, {}, "effort", self.VALID, "high"),
        "high")

  def test_invalid_persist_falls_back_to_default(self):
    self.assertEqual(
        prefs.resolve_pref(None, {"effort": "x"}, "effort", self.VALID, "high"),
        "high")


if __name__ == "__main__":
  unittest.main()
