import argparse
import sys

from cockpit_agent import CockpitAgent
from llm_client import OpenAICompatibleLLM
from llm_config import DEFAULT_PROVIDER_ID, EFFORT_MODES, PROVIDERS, get_provider


def run_interactive(agent):
  from cli_repl import CockpitRePL
  repl = CockpitRePL(agent)
  repl.run()


def main():
  parser = argparse.ArgumentParser(description="智能座舱车载助手")
  parser.add_argument("-i", "--interactive", action="store_true",
                      help="启动交互式对话模式")
  parser.add_argument("--effort", choices=EFFORT_MODES, default="high",
                      help="设置思考深度模式 (默认: high)")
  parser.add_argument("--provider", default=DEFAULT_PROVIDER_ID,
                      choices=list(PROVIDERS.keys()),
                      help="选择 LLM 供应商 (默认: %(default)s)")
  args = parser.parse_args()

  profile = get_provider(args.provider)
  agent = CockpitAgent(
      llm_client=OpenAICompatibleLLM(
          base_url=profile["base_url"],
          api_key=profile["api_key"],
          model=profile["model"],
          provider_id=args.provider,
      ),
      provider_id=args.provider,
  )
  agent.effort = args.effort

  if args.interactive:
    run_interactive(agent)
    return

  print("=" * 50)
  print("  智能座舱车载助手 Agent 演示")
  print("  特性：固定System Prompt | 工具集批量加载 | LRU淘汰 | 安全校验")
  print("=" * 50)

  # ===== 演示多轮对话 =====
  demo_queries = [
      "有点闷",
      "再放点轻音乐",
      "有点冷，把温度调高一点",
      "还能跑多远"
  ]

  for i, query in enumerate(demo_queries, 1):
    print(f"\n{'='*50}")
    print(f"用户第{i}轮: {query}")
    reply = agent.chat(query, verbose=True)
    print(f"\n助手最终回复: {reply}")

  # ===== 查看压缩后的历史 =====
  print("\n" + "=" * 50)
  print("执行历史压缩...")
  agent.compress_history()
  print(f"压缩后消息总数: {len(agent.messages)}")
  for i, msg in enumerate(agent.messages):
    role = msg["role"]
    content = str(msg["content"])[:80] + "..." if len(str(msg["content"])) > 80 else str(msg["content"])
    print(f"  [{role}] {content}")

  print("\n演示结束。可修改main.py中的query测试更多场景。")


if __name__ == "__main__":
  main()
