#include <gtest/gtest.h>

#include "token.h"

using calculator::Token;
using calculator::TokenType;

TEST(TokenTest, NumberToken) {
  Token t{TokenType::Number, 3.14, 0};
  EXPECT_EQ(t.type, TokenType::Number);
  EXPECT_DOUBLE_EQ(t.value, 3.14);
  EXPECT_EQ(t.position, 0);
}

TEST(TokenTest, OperatorTokens) {
  Token plus{TokenType::Plus, 0, 0};
  EXPECT_EQ(plus.type, TokenType::Plus);

  Token minus{TokenType::Minus, 0, 1};
  EXPECT_EQ(minus.type, TokenType::Minus);

  Token star{TokenType::Star, 0, 2};
  EXPECT_EQ(star.type, TokenType::Star);

  Token slash{TokenType::Slash, 0, 3};
  EXPECT_EQ(slash.type, TokenType::Slash);
}

TEST(TokenTest, ParenToken) {
  Token lp{TokenType::LeftParen, 0, 0};
  EXPECT_EQ(lp.type, TokenType::LeftParen);

  Token rp{TokenType::RightParen, 0, 1};
  EXPECT_EQ(rp.type, TokenType::RightParen);
}

TEST(TokenTest, EndOfInputToken) {
  Token eoi{TokenType::EndOfInput, 0, 5};
  EXPECT_EQ(eoi.type, TokenType::EndOfInput);
  EXPECT_EQ(eoi.position, 5);
}
