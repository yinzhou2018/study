#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "input_reader.h"

using calculator::InputReader;
using calculator::StdinReader;

TEST(StdinReaderTest, ReadSingleLine) {
  std::istringstream in("hello\n");
  StdinReader reader(in);
  std::string line;
  ASSERT_TRUE(reader.ReadLine(line));
  EXPECT_EQ(line, "hello");
}

TEST(StdinReaderTest, ReadMultipleLines) {
  std::istringstream in("first\nsecond\n");
  StdinReader reader(in);
  std::string line;
  ASSERT_TRUE(reader.ReadLine(line));
  EXPECT_EQ(line, "first");
  ASSERT_TRUE(reader.ReadLine(line));
  EXPECT_EQ(line, "second");
}

TEST(StdinReaderTest, ReturnsFalseOnEof) {
  std::istringstream in("");
  StdinReader reader(in);
  std::string line;
  EXPECT_FALSE(reader.ReadLine(line));
}

TEST(StdinReaderTest, ReadEmptyLine) {
  std::istringstream in("\n");
  StdinReader reader(in);
  std::string line;
  ASSERT_TRUE(reader.ReadLine(line));
  EXPECT_EQ(line, "");
}
