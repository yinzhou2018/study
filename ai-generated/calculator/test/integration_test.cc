#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "repl.h"

using calculator::Repl;

TEST(IntegrationTest, FullPipeline) {
  std::istringstream in("1+2\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("= 3"), std::string::npos);
}

TEST(IntegrationTest, DivisionByZeroIntegration) {
  std::istringstream in("1/0\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("Division by zero"), std::string::npos);
}

TEST(IntegrationTest, ParenthesesIntegration) {
  std::istringstream in("(1+2)*3\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("= 9"), std::string::npos);
}

TEST(IntegrationTest, NegativeNumber) {
  std::istringstream in("-5+3\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("= -2"), std::string::npos);
}

TEST(IntegrationTest, FloatComputation) {
  std::istringstream in("3.14*2\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("= 6.28"), std::string::npos);
}

TEST(IntegrationTest, MultipleExpressions) {
  std::istringstream in("2+3\n4*5\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("= 5"), std::string::npos);
  EXPECT_NE(out.str().find("= 20"), std::string::npos);
}

TEST(IntegrationTest, IllegalCharacterIntegration) {
  std::istringstream in("1&2\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("Error"), std::string::npos);
}

TEST(IntegrationTest, EmptyLineSkipped) {
  std::istringstream in("\n1+1\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("= 2"), std::string::npos);
}

TEST(IntegrationTest, ErrorRecoveryContinues) {
  std::istringstream in("1/0\n2+3\nexit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("Division by zero"), std::string::npos);
  EXPECT_NE(out.str().find("= 5"), std::string::npos);
}

TEST(IntegrationTest, QuitExitsRepl) {
  std::istringstream in("1+1\nquit\n");
  std::ostringstream out;
  Repl repl(in, out);
  repl.Run();
  EXPECT_NE(out.str().find("= 2"), std::string::npos);
}
