#include <gtest/gtest.h>

#include "calculator/ast.h"
#include "lexer.h"
#include "parser.h"

using calculator::BinaryExpr;
using calculator::Lexer;
using calculator::NumberExpr;
using calculator::Parser;
using calculator::UnaryExpr;

TEST(ParserTest, SingleNumber) {
  Lexer lexer("42");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  ASSERT_TRUE(expr.ok());
  EXPECT_TRUE(std::holds_alternative<NumberExpr>(expr.value()));
  EXPECT_DOUBLE_EQ(std::get<NumberExpr>(expr.value()).value, 42.0);
}

TEST(ParserTest, Addition) {
  Lexer lexer("1+2");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  ASSERT_TRUE(expr.ok());
  auto& bin = *std::get<std::unique_ptr<BinaryExpr>>(expr.value());
  EXPECT_EQ(bin.op, BinaryExpr::Add);
  EXPECT_DOUBLE_EQ(std::get<NumberExpr>(bin.left).value, 1.0);
  EXPECT_DOUBLE_EQ(std::get<NumberExpr>(bin.right).value, 2.0);
}

TEST(ParserTest, Subtraction) {
  Lexer lexer("5-3");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  ASSERT_TRUE(expr.ok());
  auto& bin = *std::get<std::unique_ptr<BinaryExpr>>(expr.value());
  EXPECT_EQ(bin.op, BinaryExpr::Sub);
}

TEST(ParserTest, ParenthesizedExpression) {
  Lexer lexer("(1+2)");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  ASSERT_TRUE(expr.ok());
  auto& inner = *std::get<std::unique_ptr<BinaryExpr>>(expr.value());
  EXPECT_EQ(inner.op, BinaryExpr::Add);
}

TEST(ParserTest, MulBeforeAdd) {
  Lexer lexer("1+2*3");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  ASSERT_TRUE(expr.ok());
  auto& top = *std::get<std::unique_ptr<BinaryExpr>>(expr.value());
  EXPECT_EQ(top.op, BinaryExpr::Add);
  EXPECT_DOUBLE_EQ(std::get<NumberExpr>(top.left).value, 1.0);
  auto& mul = *std::get<std::unique_ptr<BinaryExpr>>(top.right);
  EXPECT_EQ(mul.op, BinaryExpr::Mul);
}

TEST(ParserTest, ParenChangesPrecedence) {
  Lexer lexer("(1+2)*3");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  ASSERT_TRUE(expr.ok());
  auto& top = *std::get<std::unique_ptr<BinaryExpr>>(expr.value());
  EXPECT_EQ(top.op, BinaryExpr::Mul);
  auto& add = *std::get<std::unique_ptr<BinaryExpr>>(top.left);
  EXPECT_EQ(add.op, BinaryExpr::Add);
  EXPECT_DOUBLE_EQ(std::get<NumberExpr>(top.right).value, 3.0);
}

TEST(ParserTest, MismatchedParens) {
  Lexer lexer("(1+2");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  EXPECT_FALSE(expr.ok());
  EXPECT_NE(expr.error().message.find("Mismatched parentheses"),
            std::string::npos);
}

TEST(ParserTest, IncompleteExpression) {
  Lexer lexer("1+");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  EXPECT_FALSE(expr.ok());
}
