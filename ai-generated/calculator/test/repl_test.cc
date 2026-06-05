#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "repl.h"

using calculator::Repl;

TEST(ReplTest, ExpressionOutput) {
  std::istringstream in("1+2\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("3"), std::string::npos);
}

TEST(ReplTest, ExitCommand) {
  std::istringstream in("exit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("Calculator"), std::string::npos);
}

TEST(ReplTest, QuitCommand) {
  std::istringstream in("quit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("Calculator"), std::string::npos);
}

TEST(ReplTest, ErrorContinues) {
  std::istringstream in("1++2\n2+3\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("Error"), std::string::npos);
  EXPECT_NE(out.str().find("5"), std::string::npos);
}

TEST(ReplTest, EmptyLineIgnored) {
  std::istringstream in("\n1+1\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("2"), std::string::npos);
}

TEST(ReplTest, WelcomeMessage) {
  std::istringstream in("exit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("Calculator"), std::string::npos);
  EXPECT_NE(out.str().find("quit"), std::string::npos);
}
