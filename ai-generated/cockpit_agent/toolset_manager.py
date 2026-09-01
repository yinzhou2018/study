import json
from collections import OrderedDict

from config import MAX_ACTIVE_TOOLSETS, build_system_tools


class ToolsetManager:
  def __init__(self, config_path="toolsets.json"):
    self.max_active = MAX_ACTIVE_TOOLSETS
    # 按使用顺序存储，LRU淘汰
    self.active_toolsets = OrderedDict()
    # 加载全量工具集配置
    with open(config_path, "r", encoding="utf-8") as f:
      self.all_toolsets_config = json.load(f)

  def is_valid_toolset(self, toolset_id: str) -> bool:
    """校验工具集ID是否合法"""
    return toolset_id in self.all_toolsets_config

  def load_toolsets(self, toolset_ids: list) -> dict:
    """批量加载工具集，返回结构化结果"""
    loaded = []
    failed = []

    for toolset_id in toolset_ids:
      if not self.is_valid_toolset(toolset_id):
        failed.append({
            "toolset_id": toolset_id,
            "reason": "工具集ID不存在"
        })
        continue

      # 已激活则刷新使用顺序
      if toolset_id in self.active_toolsets:
        self.active_toolsets.move_to_end(toolset_id)
        loaded.append(self._build_load_result(toolset_id))
        continue

      # 超上限则淘汰最久未使用的
      while len(self.active_toolsets) >= self.max_active:
        oldest_id = next(iter(self.active_toolsets))
        del self.active_toolsets[oldest_id]

      # 加载新工具集
      cfg = self.all_toolsets_config[toolset_id]
      self.active_toolsets[toolset_id] = cfg["tools"]
      loaded.append(self._build_load_result(toolset_id))

    status = "success" if not failed else "partial_success"
    return {
        "status": status,
        "loaded": loaded,
        "failed": failed,
        "current_active_count": len(self.active_toolsets),
        "message": f"成功加载{len(loaded)}个，失败{len(failed)}个"
    }

  def get_current_tools(self) -> list:
    """动态组装当前完整工具列表（系统工具 + 已激活业务工具）"""
    tools = self.get_system_tools()
    for tool_defs in self.active_toolsets.values():
      tools.extend(tool_defs)
    return tools

  def get_system_tools(self) -> list:
    """构建系统工具,enum 由 toolsets.json 全量工具集ID动态生成"""
    return build_system_tools(list(self.all_toolsets_config.keys()))

  def get_active_toolset_ids(self) -> list:
    return list(self.active_toolsets.keys())

  def get_toolset_listing(self) -> str:
    """生成全量工具集描述,供System Prompt使用"""
    lines = []
    for toolset_id, cfg in self.all_toolsets_config.items():
      lines.append(f"- {toolset_id}:{cfg['name']},{cfg['description']}")
    return "\n".join(lines)

  def _build_load_result(self, toolset_id: str) -> dict:
    cfg = self.all_toolsets_config[toolset_id]
    return {
        "toolset_id": toolset_id,
        "toolset_name": cfg["name"],
        "available_tools": len(cfg["tools"]),
        "execution_rules": cfg.get("execution_rules", [])
    }
