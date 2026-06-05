#include <gtest/gtest.h>

#include "result.h"

using calculator::Error;
using calculator::Result;

TEST(ResultTest, HoldsValue) {
  Result<int> r(42);
  EXPECT_TRUE(r.ok());
  EXPECT_EQ(r.value(), 42);
}

TEST(ResultTest, HoldsError) {
  Result<int> r(Error{"something went wrong"});
  EXPECT_FALSE(r.ok());
  EXPECT_EQ(r.error().message, "something went wrong");
}

TEST(ResultTest, TakeMovesValue) {
  Result<std::string> r(std::string{"hello"});
  EXPECT_TRUE(r.ok());
  auto val = r.Take();
  EXPECT_EQ(val, "hello");
}
