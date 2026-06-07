#include <gtest/gtest.h>

#include "ast.h"
#include "evaluator.h"

using calculator::BinaryExpr;
using calculator::ConstantExpr;
using calculator::Expr;
using calculator::FuncCallExpr;
using calculator::NumberExpr;
using calculator::UnaryExpr;
using calculator::evaluator::Evaluate;

TEST(EvaluatorTest, Number) {
  auto result = Evaluate(NumberExpr{42.0});
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 42.0);
}

TEST(EvaluatorTest, Addition) {
  auto node = std::make_unique<BinaryExpr>();
  node->op = BinaryExpr::Add;
  node->left = NumberExpr{1.0};
  node->right = NumberExpr{2.0};
  auto result = Evaluate(Expr(std::move(node)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 3.0);
}

TEST(EvaluatorTest, Subtraction) {
  auto node = std::make_unique<BinaryExpr>();
  node->op = BinaryExpr::Sub;
  node->left = NumberExpr{5.0};
  node->right = NumberExpr{3.0};
  auto result = Evaluate(Expr(std::move(node)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 2.0);
}

TEST(EvaluatorTest, Multiplication) {
  auto node = std::make_unique<BinaryExpr>();
  node->op = BinaryExpr::Mul;
  node->left = NumberExpr{4.0};
  node->right = NumberExpr{3.0};
  auto result = Evaluate(Expr(std::move(node)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 12.0);
}

TEST(EvaluatorTest, Division) {
  auto node = std::make_unique<BinaryExpr>();
  node->op = BinaryExpr::Div;
  node->left = NumberExpr{10.0};
  node->right = NumberExpr{2.0};
  auto result = Evaluate(Expr(std::move(node)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 5.0);
}

TEST(EvaluatorTest, CompoundExpression) {
  auto inner = std::make_unique<BinaryExpr>();
  inner->op = BinaryExpr::Add;
  inner->left = NumberExpr{1.0};
  inner->right = NumberExpr{2.0};

  auto outer = std::make_unique<BinaryExpr>();
  outer->op = BinaryExpr::Mul;
  outer->left = NumberExpr{3.0};
  outer->right = Expr(std::move(inner));

  auto result = Evaluate(Expr(std::move(outer)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 9.0);
}

TEST(EvaluatorTest, DivisionByZero) {
  auto node = std::make_unique<BinaryExpr>();
  node->op = BinaryExpr::Div;
  node->left = NumberExpr{1.0};
  node->right = NumberExpr{0.0};
  auto result = Evaluate(Expr(std::move(node)));
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("Division by zero"), std::string::npos);
}

TEST(EvaluatorTest, UnaryNegate) {
  auto node = std::make_unique<UnaryExpr>();
  node->op = UnaryExpr::Negate;
  node->operand = NumberExpr{5.0};
  auto result = Evaluate(Expr(std::move(node)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), -5.0);
}

TEST(EvaluatorTest, SinFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "sin";
  func->args.push_back(NumberExpr{1.5707963267948966});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_NEAR(result.value(), 1.0, 1e-9);
}

TEST(EvaluatorTest, CosFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "cos";
  func->args.push_back(NumberExpr{0.0});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 1.0);
}

TEST(EvaluatorTest, TanFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "tan";
  func->args.push_back(NumberExpr{0.7853981633974483});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_NEAR(result.value(), 1.0, 1e-9);
}

TEST(EvaluatorTest, AsinFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "asin";
  func->args.push_back(NumberExpr{0.5});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_NEAR(result.value(), 0.5235987755982989, 1e-9);
}

TEST(EvaluatorTest, AcosFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "acos";
  func->args.push_back(NumberExpr{0.5});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_NEAR(result.value(), 1.0471975511965979, 1e-9);
}

TEST(EvaluatorTest, AtanFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "atan";
  func->args.push_back(NumberExpr{1.0});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_NEAR(result.value(), 0.7853981633974483, 1e-9);
}

TEST(EvaluatorTest, SqrtFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "sqrt";
  func->args.push_back(NumberExpr{4.0});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 2.0);
}

TEST(EvaluatorTest, LogFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "log";
  func->args.push_back(NumberExpr{100.0});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 2.0);
}

TEST(EvaluatorTest, LnFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "ln";
  func->args.push_back(NumberExpr{2.718281828459045});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_NEAR(result.value(), 1.0, 1e-9);
}

TEST(EvaluatorTest, AbsFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "abs";
  func->args.push_back(NumberExpr{-3.5});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 3.5);
}

TEST(EvaluatorTest, CeilFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "ceil";
  func->args.push_back(NumberExpr{3.2});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 4.0);
}

TEST(EvaluatorTest, FloorFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "floor";
  func->args.push_back(NumberExpr{3.8});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 3.0);
}

TEST(EvaluatorTest, PowFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "pow";
  func->args.push_back(NumberExpr{2.0});
  func->args.push_back(NumberExpr{3.0});
  auto result = Evaluate(Expr(std::move(func)));
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 8.0);
}

TEST(EvaluatorTest, ConstantPi) {
  auto result = Evaluate(ConstantExpr{"pi"});
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 3.141592653589793);
}

TEST(EvaluatorTest, ConstantE) {
  auto result = Evaluate(ConstantExpr{"e"});
  ASSERT_TRUE(result.ok());
  EXPECT_DOUBLE_EQ(result.value(), 2.718281828459045);
}

TEST(EvaluatorTest, SqrtNegativeDomainError) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "sqrt";
  func->args.push_back(NumberExpr{-1.0});
  auto result = Evaluate(Expr(std::move(func)));
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("non-negative"), std::string::npos);
}

TEST(EvaluatorTest, LogZeroDomainError) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "log";
  func->args.push_back(NumberExpr{0.0});
  auto result = Evaluate(Expr(std::move(func)));
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("positive"), std::string::npos);
}

TEST(EvaluatorTest, LnNegativeDomainError) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "ln";
  func->args.push_back(NumberExpr{-1.0});
  auto result = Evaluate(Expr(std::move(func)));
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("positive"), std::string::npos);
}

TEST(EvaluatorTest, AsinOutOfRangeDomainError) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "asin";
  func->args.push_back(NumberExpr{2.0});
  auto result = Evaluate(Expr(std::move(func)));
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("-1 and 1"), std::string::npos);
}

TEST(EvaluatorTest, AcosOutOfRangeDomainError) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "acos";
  func->args.push_back(NumberExpr{-1.5});
  auto result = Evaluate(Expr(std::move(func)));
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("-1 and 1"), std::string::npos);
}

TEST(EvaluatorTest, WrongArgCount) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "sin";
  func->args.push_back(NumberExpr{1.0});
  func->args.push_back(NumberExpr{2.0});
  auto result = Evaluate(Expr(std::move(func)));
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("1 argument"), std::string::npos);
}

TEST(EvaluatorTest, PowWrongArgCount) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "pow";
  func->args.push_back(NumberExpr{2.0});
  auto result = Evaluate(Expr(std::move(func)));
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("2 argument"), std::string::npos);
}

TEST(EvaluatorTest, UnknownFunction) {
  auto func = std::make_unique<FuncCallExpr>();
  func->name = "unknown";
  func->args.push_back(NumberExpr{1.0});
  auto result = Evaluate(Expr(std::move(func)));
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("Unknown function"), std::string::npos);
}

TEST(EvaluatorTest, UnknownConstant) {
  auto result = Evaluate(ConstantExpr{"unknown"});
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("Unknown constant"), std::string::npos);
}
