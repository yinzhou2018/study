#!/bin/bash

# 1. 从 stdin 读取 Claude Code 传来的 JSON 数据
INPUT=$(cat)
FILE_PATH=$(echo "$INPUT" | jq -r '.tool_input.file_path // empty')

# 2. 定义需要格式化的 C/C++ 文件后缀
CPP_EXTENSIONS="\\.(c|cpp|cc|cxx|h|hpp|hxx|hh)$"

# 3. 判断文件后缀并执行格式化
if [[ "$FILE_PATH" =~ $CPP_EXTENSIONS ]]; then
  # 必须将日志输出到 stderr，绝不能输出到 stdout
  echo "🔍 [Hook] 检测到 C/C++ 文件，正在格式化: $FILE_PATH" >&2 
  
  # 执行 clang-format
  clang-format -i "$FILE_PATH"
  
  # 可选：检查格式化是否成功
  if [ $? -ne 0 ]; then
    echo "❌ [Hook] clang-format 执行失败!" >&2
    exit 1
  fi
fi

# 4. 必须原样将接收到的 JSON 输出到 stdout，告诉 Claude Code 一切正常
echo "$INPUT"