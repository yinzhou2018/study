#include <gtest/gtest.h>

#include "calculator/ast.h"

using calculator::BinaryExpr;
using calculator::Expr;
using calculator::NumberExpr;
using calculator::UnaryExpr;

TEST(AstTest, NumberExpr) {
  Expr expr = Expr(NumberExpr{42.0});
  EXPECT_TRUE(std::holds_alternative<NumberExpr>(expr));
  EXPECT_DOUBLE_EQ(std::get<NumberExpr>(expr).value, 42.0);
}

TEST(AstTest, BinaryExpr) {
  auto node = std::make_unique<BinaryExpr>();
  node->op = BinaryExpr::Add;
  node->left = Expr(NumberExpr{1.0});
  node->right = Expr(NumberExpr{2.0});
  Expr expr(std::move(node));

  EXPECT_TRUE(std::holds_alternative<std::unique_ptr<BinaryExpr>>(expr));
  auto& bin = *std::get<std::unique_ptr<BinaryExpr>>(expr);
  EXPECT_EQ(bin.op, BinaryExpr::Add);
  EXPECT_TRUE(std::holds_alternative<NumberExpr>(bin.left));
  EXPECT_TRUE(std::holds_alternative<NumberExpr>(bin.right));
}

TEST(AstTest, UnaryExpr) {
  auto node = std::make_unique<UnaryExpr>();
  node->op = UnaryExpr::Negate;
  node->operand = Expr(NumberExpr{5.0});
  Expr expr(std::move(node));

  EXPECT_TRUE(std::holds_alternative<std::unique_ptr<UnaryExpr>>(expr));
  auto& un = *std::get<std::unique_ptr<UnaryExpr>>(expr);
  EXPECT_EQ(un.op, UnaryExpr::Negate);
  EXPECT_TRUE(std::holds_alternative<NumberExpr>(un.operand));
}

TEST(AstTest, NestedBinaryExpr) {
  auto inner = std::make_unique<BinaryExpr>();
  inner->op = BinaryExpr::Mul;
  inner->left = Expr(NumberExpr{2.0});
  inner->right = Expr(NumberExpr{3.0});

  auto outer = std::make_unique<BinaryExpr>();
  outer->op = BinaryExpr::Add;
  outer->left = Expr(NumberExpr{1.0});
  outer->right = Expr(std::move(inner));

  Expr expr(std::move(outer));
  auto& bin = *std::get<std::unique_ptr<BinaryExpr>>(expr);
  EXPECT_EQ(bin.op, BinaryExpr::Add);
  EXPECT_TRUE(std::holds_alternative<std::unique_ptr<BinaryExpr>>(bin.right));
}
