#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <string>

#include "evaluator.h"
#include "input_reader.h"
#include "repl.h"

using calculator::Repl;
using calculator::StdinReader;
using calculator::evaluator::EvaluateExpression;

TEST(IntegrationTest, FullPipeline) {
  std::istringstream in("1+2\nexit\n");
  std::ostringstream out;
  auto reader = std::make_unique<StdinReader>(in);
  Repl repl(std::move(reader), out);
  repl.Run();
  EXPECT_NE(out.str().find("3"), std::string::npos);
}

TEST(IntegrationTest, DivisionByZeroIntegration) {
  std::istringstream in("1/0\nexit\n");
  std::ostringstream out;
  auto reader = std::make_unique<StdinReader>(in);
  Repl repl(std::move(reader), out);
  repl.Run();
  EXPECT_NE(out.str().find("Division by zero"), std::string::npos);
}

TEST(IntegrationTest, ParenthesesIntegration) {
  std::istringstream in("(1+2)*3\nexit\n");
  std::ostringstream out;
  auto reader = std::make_unique<StdinReader>(in);
  Repl repl(std::move(reader), out);
  repl.Run();
  EXPECT_NE(out.str().find("9"), std::string::npos);
}

TEST(IntegrationTest, NegativeNumber) {
  std::istringstream in("-5+3\nexit\n");
  std::ostringstream out;
  auto reader = std::make_unique<StdinReader>(in);
  Repl repl(std::move(reader), out);
  repl.Run();
  EXPECT_NE(out.str().find("-2"), std::string::npos);
}

TEST(IntegrationTest, FloatComputation) {
  std::istringstream in("3.14*2\nexit\n");
  std::ostringstream out;
  auto reader = std::make_unique<StdinReader>(in);
  Repl repl(std::move(reader), out);
  repl.Run();
  EXPECT_NE(out.str().find("6.28"), std::string::npos);
}

TEST(IntegrationTest, MultipleExpressions) {
  std::istringstream in("2+3\n4*5\nexit\n");
  std::ostringstream out;
  auto reader = std::make_unique<StdinReader>(in);
  Repl repl(std::move(reader), out);
  repl.Run();
  EXPECT_NE(out.str().find("5"), std::string::npos);
  EXPECT_NE(out.str().find("20"), std::string::npos);
}

TEST(IntegrationTest, IllegalCharacterIntegration) {
  std::istringstream in("1&2\nexit\n");
  std::ostringstream out;
  auto reader = std::make_unique<StdinReader>(in);
  Repl repl(std::move(reader), out);
  repl.Run();
  EXPECT_NE(out.str().find("Error"), std::string::npos);
}

TEST(IntegrationTest, EmptyLineSkipped) {
  std::istringstream in("\n1+1\nexit\n");
  std::ostringstream out;
  auto reader = std::make_unique<StdinReader>(in);
  Repl repl(std::move(reader), out);
  repl.Run();
  EXPECT_NE(out.str().find("2"), std::string::npos);
}

TEST(IntegrationTest, ErrorRecoveryContinues) {
  std::istringstream in("1/0\n2+3\nexit\n");
  std::ostringstream out;
  auto reader = std::make_unique<StdinReader>(in);
  Repl repl(std::move(reader), out);
  repl.Run();
  EXPECT_NE(out.str().find("Division by zero"), std::string::npos);
  EXPECT_NE(out.str().find("5"), std::string::npos);
}

TEST(IntegrationTest, QuitExitsRepl) {
  std::istringstream in("1+1\nquit\n");
  std::ostringstream out;
  auto reader = std::make_unique<StdinReader>(in);
  Repl repl(std::move(reader), out);
  repl.Run();
  EXPECT_NE(out.str().find("2"), std::string::npos);
}

TEST(EvaluateExpressionIntegrationTest, SingleArgExpression) {
  auto result = EvaluateExpression("1+2");
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 3.0);
}

TEST(EvaluateExpressionIntegrationTest, MultiArgJoined) {
  auto result = EvaluateExpression("1 + 2");
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 3.0);
}

TEST(EvaluateExpressionIntegrationTest, ErrorOnDivisionByZero) {
  auto result = EvaluateExpression("1/0");
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("Division by zero"), std::string::npos);
}

TEST(EvaluateExpressionIntegrationTest, ErrorOnLexError) {
  auto result = EvaluateExpression("1&2");
  EXPECT_FALSE(result.ok());
}

TEST(EvaluateExpressionIntegrationTest, ErrorOnParseError) {
  auto result = EvaluateExpression("1++2");
  EXPECT_FALSE(result.ok());
}
