#include <gtest/gtest.h>

#include "lexer.h"

using calculator::Lexer;
using calculator::Token;
using calculator::TokenType;

TEST(LexerTest, SingleNumber) {
  Lexer lexer("42");
  auto result = lexer.Tokenize();
  ASSERT_TRUE(result.ok());
  auto& tokens = result.value();
  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens[0].type, TokenType::Number);
  EXPECT_DOUBLE_EQ(tokens[0].value, 42.0);
  EXPECT_EQ(tokens[1].type, TokenType::EndOfInput);
}

TEST(LexerTest, FloatNumber) {
  Lexer lexer("3.14");
  auto result = lexer.Tokenize();
  ASSERT_TRUE(result.ok());
  auto& tokens = result.value();
  EXPECT_EQ(tokens[0].type, TokenType::Number);
  EXPECT_DOUBLE_EQ(tokens[0].value, 3.14);
}

TEST(LexerTest, Operators) {
  Lexer lexer("1+2-3*4/5");
  auto result = lexer.Tokenize();
  ASSERT_TRUE(result.ok());
  auto& tokens = result.value();
  ASSERT_EQ(tokens.size(), 10u);
  EXPECT_EQ(tokens[1].type, TokenType::Plus);
  EXPECT_EQ(tokens[3].type, TokenType::Minus);
  EXPECT_EQ(tokens[5].type, TokenType::Star);
  EXPECT_EQ(tokens[7].type, TokenType::Slash);
}

TEST(LexerTest, Parentheses) {
  Lexer lexer("(1+2)");
  auto result = lexer.Tokenize();
  ASSERT_TRUE(result.ok());
  auto& tokens = result.value();
  ASSERT_EQ(tokens.size(), 6u);
  EXPECT_EQ(tokens[0].type, TokenType::LeftParen);
  EXPECT_EQ(tokens[4].type, TokenType::RightParen);
}

TEST(LexerTest, UnaryMinus) {
  Lexer lexer("-5");
  auto result = lexer.Tokenize();
  ASSERT_TRUE(result.ok());
  auto& tokens = result.value();
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0].type, TokenType::Minus);
  EXPECT_EQ(tokens[1].type, TokenType::Number);
  EXPECT_DOUBLE_EQ(tokens[1].value, 5.0);
}

TEST(LexerTest, WhitespaceSkipped) {
  Lexer lexer("  1  +  2  ");
  auto result = lexer.Tokenize();
  ASSERT_TRUE(result.ok());
  auto& tokens = result.value();
  ASSERT_EQ(tokens.size(), 4u);
  EXPECT_EQ(tokens[0].type, TokenType::Number);
  EXPECT_EQ(tokens[1].type, TokenType::Plus);
  EXPECT_EQ(tokens[2].type, TokenType::Number);
}

TEST(LexerTest, ComplexExpression) {
  Lexer lexer("(1+2)*3");
  auto result = lexer.Tokenize();
  ASSERT_TRUE(result.ok());
  auto& tokens = result.value();
  ASSERT_EQ(tokens.size(), 8u);
  EXPECT_EQ(tokens[0].type, TokenType::LeftParen);
  EXPECT_EQ(tokens[1].type, TokenType::Number);
  EXPECT_EQ(tokens[2].type, TokenType::Plus);
  EXPECT_EQ(tokens[3].type, TokenType::Number);
  EXPECT_EQ(tokens[4].type, TokenType::RightParen);
  EXPECT_EQ(tokens[5].type, TokenType::Star);
  EXPECT_EQ(tokens[6].type, TokenType::Number);
}

TEST(LexerTest, IllegalCharacter) {
  Lexer lexer("1&2");
  auto result = lexer.Tokenize();
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("Illegal character"),
            std::string::npos);
}

TEST(LexerTest, IllegalCharacterPosition) {
  Lexer lexer("1+@3");
  auto result = lexer.Tokenize();
  EXPECT_FALSE(result.ok());
  EXPECT_NE(result.error().message.find("position 2"), std::string::npos);
}
