#pragma once
#include <algorithm>
#include <fstream>
#include <vector>

bool preload_file(const char* file_path) {
  std::cout << "Ready to preload file: " << file_path << std::endl;
  std::ifstream file(file_path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    std::cout << "Unable to open file." << std::endl;
    return false;
  }

  auto file_size = file.tellg();
  std::cout << "File size: " << file_size << std::endl;
  file.seekg(0, std::ios::beg);

  std::cout << "Fragment data: ";
  auto chunk_size = 1024 * 1024;  // 1M
  std::vector<char> buffer(chunk_size);
  auto total_read_size = 0;
  while (total_read_size < file_size) {
    file.read(buffer.data(), buffer.size());
    auto read_size = file.gcount();
    if (read_size <= 0) {
      break;
    }

    // 每次在控制台输出开头几个字节方便调试
    auto show_count = read_size > 5 ? 5 : read_size;
    std::for_each_n(buffer.begin(), show_count, [](auto elem) { std::cout << (int)elem << " "; });
  }

  return true;
}