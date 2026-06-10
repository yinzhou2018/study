#include <gtest/gtest.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"

using calculator::BinaryExpr;
using calculator::ConstantExpr;
using calculator::FuncCallExpr;
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

TEST(ParserTest, SingleArgFunctionCall) {
  Lexer lexer("sin(1)");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  ASSERT_TRUE(expr.ok());
  auto& func = *std::get<std::unique_ptr<FuncCallExpr>>(expr.value());
  EXPECT_EQ(func.name, "sin");
  ASSERT_EQ(func.args.size(), 1u);
  EXPECT_DOUBLE_EQ(std::get<NumberExpr>(func.args[0]).value, 1.0);
}

TEST(ParserTest, TwoArgFunctionCall) {
  Lexer lexer("pow(2,3)");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  ASSERT_TRUE(expr.ok());
  auto& func = *std::get<std::unique_ptr<FuncCallExpr>>(expr.value());
  EXPECT_EQ(func.name, "pow");
  ASSERT_EQ(func.args.size(), 2u);
  EXPECT_DOUBLE_EQ(std::get<NumberExpr>(func.args[0]).value, 2.0);
  EXPECT_DOUBLE_EQ(std::get<NumberExpr>(func.args[1]).value, 3.0);
}

TEST(ParserTest, ConstantReference) {
  Lexer lexer("pi");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  ASSERT_TRUE(expr.ok());
  auto& con = std::get<ConstantExpr>(expr.value());
  EXPECT_EQ(con.name, "pi");
}

TEST(ParserTest, FunctionCallPrecedence) {
  Lexer lexer("2*sin(1)");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  ASSERT_TRUE(expr.ok());
  auto& mul = *std::get<std::unique_ptr<BinaryExpr>>(expr.value());
  EXPECT_EQ(mul.op, BinaryExpr::Mul);
  EXPECT_DOUBLE_EQ(std::get<NumberExpr>(mul.left).value, 2.0);
  auto& func = *std::get<std::unique_ptr<FuncCallExpr>>(mul.right);
  EXPECT_EQ(func.name, "sin");
}

TEST(ParserTest, FunctionCallMismatchedParens) {
  Lexer lexer("sin(1");
  auto tokens = lexer.Tokenize();
  ASSERT_TRUE(tokens.ok());
  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  EXPECT_FALSE(expr.ok());
  EXPECT_NE(expr.error().message.find("Mismatched parentheses"),
            std::string::npos);
}
