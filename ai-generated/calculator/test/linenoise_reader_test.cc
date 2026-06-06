#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include <linenoise.h>

#include "input_reader.h"

using calculator::LinenoiseReader;

TEST(LinenoiseReaderTest, HistoryFileCreation) {
  std::string tmpdir = std::filesystem::temp_directory_path().string();
  std::string history_path = tmpdir + "/test_calculator_history";

  std::remove(history_path.c_str());

  linenoiseHistoryAdd("1+2");
  linenoiseHistorySave(history_path.c_str());

  ASSERT_TRUE(std::filesystem::exists(history_path));

  std::ifstream f(history_path);
  std::string content;
  std::getline(f, content);
  EXPECT_EQ(content, "1+2");

  std::remove(history_path.c_str());
}

TEST(LinenoiseReaderTest, LoadNonexistentHistoryFile) {
  std::string tmpdir = std::filesystem::temp_directory_path().string();
  std::string history_path = tmpdir + "/nonexistent_history_12345";

  std::remove(history_path.c_str());

  linenoiseHistoryLoad(history_path.c_str());

  std::remove(history_path.c_str());
  SUCCEED();
}
