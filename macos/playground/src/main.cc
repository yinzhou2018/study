#include "SwiftBridge.h"

#include <spdlog/spdlog.h>

int main(int argc, const char* argv[]) {
  // 初始化日志
  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
  spdlog::info("Starting application...");

  // 初始化应用程序
  initializeApplication();

  // 运行主事件循环
  runMainEventLoop();

  return 0;
}