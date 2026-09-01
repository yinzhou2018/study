# 全程固定不变的System Prompt,最大化Prefix KV Cache收益
# 全量工具集列表由 ToolsetManager 从 toolsets.json 动态生成,此处仅保留模板
SYSTEM_PROMPT_TEMPLATE = """
## 角色定义
你是智能座舱车载助手,在持续对话中通过调用工具为用户提供车辆控制、信息查询、娱乐导航等服务,回复简洁自然,适合语音播报,不要提及技术术语。

## 核心基础规则
- 基于用户请求按需调用load_toolsets加载对应工具集,加载成功后才可使用该工具集内的业务工具。
- 同一时间最多可激活3个工具集,加载第4个时会自动替换最久未使用的工具集。
- 只能使用当前已激活工具集内的工具,禁止使用未加载的能力,禁止自创工具和参数。
- 同一话题下的追问,复用已激活的工具集,不要重复加载。

## 系统工具: 永久可用
- load_toolsets:批量加载并激活指定工具集,成功后获得对应业务工具权限,最多同时加载3个
- list_active_toolsets:查询当前已激活的所有工具集

## 全量工具集
{toolset_listing}

## 回复要求
- 自然口语化,简短清晰,适合开车时收听
- 不确定的操作直接询问用户,不要猜测
"""


def build_system_prompt(toolset_listing: str) -> str:
  """将工具集列表填入模板,生成完整System Prompt"""
  return SYSTEM_PROMPT_TEMPLATE.format(toolset_listing=toolset_listing)

# 系统工具定义（常驻）,toolset_ids 枚举由 ToolsetManager 从 toolsets.json 动态传入
def build_system_tools(toolset_ids: list) -> list:
  """根据全量工具集ID构建系统工具,确保 enum 与 toolsets.json 保持一致"""
  return [
      {
          "type": "function",
          "function": {
              "name": "load_toolsets",
              "description": "批量加载并激活指定的车载工具集,加载成功后可使用对应工具集内的所有业务工具。同一时间最多激活3个工具集,超出数量会自动替换最久未使用的工具集。",
              "parameters": {
                  "type": "object",
                  "properties": {
                      "toolset_ids": {
                          "type": "array",
                          "items": {
                              "type": "string",
                              "enum": toolset_ids
                          },
                          "minItems": 1,
                          "maxItems": 3,
                          "description": "要加载的工具集ID列表,最多传入3个"
                      }
                  },
                  "required": ["toolset_ids"]
              }
          }
      },
      {
          "type": "function",
          "function": {
              "name": "list_active_toolsets",
              "description": "查询当前已激活的所有工具集及对应可用能力",
              "parameters": {
                  "type": "object",
                  "properties": {},
                  "required": []
              }
          }
      }
  ]

# 最大同时激活工具集数量
MAX_ACTIVE_TOOLSETS = 3

# 模拟车辆实时状态（真实环境从车机总线获取）
MOCK_VEHICLE_STATE = {
    "speed": 42,  # km/h
    "outside_temp": 28,
    "inside_temp": 26,
    "range": 380,  # km
    "window_left_front": 0,
    "sunroof_tilt": "off",
    "ac_circulation": "inner",
    "ac_temperature": 24,
    "media_volume": 15
}
