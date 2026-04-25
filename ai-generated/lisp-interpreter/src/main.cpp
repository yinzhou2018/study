#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "eval.hpp"

// 函数声明
std::string removeComments(const std::string& input);

// 声明解析器的全局变量和函数
extern FILE* yyin;
extern int yyparse();
extern ValuePtr parsedResult;

// 检查输入是否只包含空白和注释
bool isOnlyWhitespaceOrComment(const std::string& input) {
  std::string withoutComments = removeComments(input);
  return withoutComments.find_first_not_of(" \t\n\r") == std::string::npos;
}

// 执行单个表达式
ValuePtr evalExpression(const std::string& input, std::shared_ptr<Env>& env) {
  // 如果只有空白/注释，直接返回 nil
  if (isOnlyWhitespaceOrComment(input)) {
    return nil();
  }

  yyin = fmemopen(const_cast<char*>(input.c_str()), input.length(), "r");
  if (yyparse() != 0) {
    if (yyin)
      fclose(yyin);
    throw std::runtime_error("Parse error");
  }
  if (yyin)
    fclose(yyin);

  if (parsedResult) {
    return eval(parsedResult, env);
  }
  return nil();
}

// 去除字符串首尾空白
std::string trim(const std::string& s) {
  auto start = s.find_first_not_of(" \t\n\r");
  if (start == std::string::npos)
    return "";
  auto end = s.find_last_not_of(" \t\n\r");
  return s.substr(start, end - start + 1);
}

// 移除注释，返回纯代码
std::string removeComments(const std::string& input) {
  std::string result;
  result.reserve(input.size()); // 预分配内存

  size_t start = 0;
  size_t commentPos = input.find(';');
  while (commentPos != std::string::npos) {
    result.append(input, start, commentPos - start);
    start = input.find('\n', commentPos);
    if (start != std::string::npos) {
      result.push_back('\n');
      start++;
    }
    commentPos = input.find(';', start);
  }
  result.append(input, start, std::string::npos);
  return result;
}

// 检查文件是否有根括号包裹（忽略首尾空白和注释）
bool hasRootParentheses(const std::string& code) {
  std::string trimmed = trim(code);
  if (trimmed.empty() || trimmed[0] != '(')
    return false;

  // 检查外层括号是否包裹整个文件
  int parenCount = 0;
  for (size_t i = 0; i < trimmed.length(); ++i) {
    if (trimmed[i] == '(')
      parenCount++;
    else if (trimmed[i] == ')')
      parenCount--;

    // 如果在最外层括号闭合后还有非空白内容，则不是根括号
    if (parenCount == 0 && i < trimmed.length() - 1)
      return false;
  }
  return parenCount == 0;
}

// 执行有根括号包裹的文件
int runFileWithRootParentheses(const std::string& filename, std::shared_ptr<Env>& env) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
    return 1;
  }

  // 读取整个文件
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  file.close();

  std::string codeOnly = removeComments(content);
  std::string trimmedCode = trim(codeOnly);

  yyin = fmemopen(const_cast<char*>(trimmedCode.c_str()), trimmedCode.length(), "r");
  if (yyparse() != 0) {
    if (yyin)
      fclose(yyin);
    std::cerr << "Error: Parse error" << std::endl;
    return 1;
  }
  if (yyin)
    fclose(yyin);

  if (parsedResult && parsedResult->isList()) {
    auto list = std::dynamic_pointer_cast<List>(parsedResult);
    for (const auto& expr : list->get()) {
      try {
        eval(expr, env);
      } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
      }
    }
  }

  return 0;
}

// 执行无根括号包裹的文件
int runFileWithoutRootParentheses(const std::string& filename, std::shared_ptr<Env>& env) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
    return 1;
  }

  // 读取整个文件
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  file.close();

  std::istringstream stream(content);
  std::string line;
  std::string input;
  int parenCount = 0;

  while (std::getline(stream, line)) {
    // 计算括号平衡（忽略注释部分）
    size_t commentPos = line.find(';');
    std::string codePart = (commentPos != std::string::npos) ? line.substr(0, commentPos) : line;

    for (char c : codePart) {
      if (c == '(')
        parenCount++;
      if (c == ')')
        parenCount--;
    }
    input += line + "\n";

    // 当括号平衡时执行
    if (parenCount == 0 && !input.empty()) {
      try {
        evalExpression(input, env);
      } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
      }
      input.clear();
    }
  }

  // 处理未完成的输入（文件末尾括号不完整）
  if (parenCount != 0) {
    std::cerr << "Error: Unbalanced parentheses" << std::endl;
    return 1;
  }

  return 0;
}

// 从文件执行代码
int runFile(const char* filename) {
  auto env = createGlobalEnv();

  // 检查是否有根括号包裹
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
    return 1;
  }

  // 读取整个文件
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  file.close();

  std::string codeOnly = removeComments(content);
  std::string trimmedCode = trim(codeOnly);

  if (hasRootParentheses(trimmedCode)) {
    return runFileWithRootParentheses(filename, env);
  } else {
    return runFileWithoutRootParentheses(filename, env);
  }
}

// REPL 模式
int runREPL() {
  std::cout << "Lisp Interpreter v1.0" << std::endl;
  std::cout << "Type 'quit' to exit" << std::endl;
  std::cout << std::endl;

  auto env = createGlobalEnv();

  while (true) {
    std::cout << "> ";
    std::cout.flush();

    std::string line;
    if (!std::getline(std::cin, line)) {
      break;
    }

    // 跳过空行
    if (line.empty())
      continue;

    // 退出命令
    if (line == "quit" || line == "exit") {
      break;
    }

    // 解析输入
    std::string input = line;
    // 继续读取直到括号匹配
    int parenCount = 0;
    for (char c : line) {
      if (c == '(')
        parenCount++;
      if (c == ')')
        parenCount--;
    }
    while (parenCount > 0) {
      std::cout << "  ";
      std::cout.flush();
      if (!std::getline(std::cin, line))
        break;
      input += "\n" + line;
      for (char c : line) {
        if (c == '(')
          parenCount++;
        if (c == ')')
          parenCount--;
      }
    }

    try {
      auto result = evalExpression(input, env);
      std::cout << result->toString() << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }

  std::cout << "Bye!" << std::endl;
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    // 文件执行模式
    return runFile(argv[1]);
  } else {
    // REPL 模式
    return runREPL();
  }
}