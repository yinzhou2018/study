#include <gtest/gtest.h>

#include "calculator/ast.h"
#include "evaluator.h"

using calculator::BinaryExpr;
using calculator::Expr;
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
