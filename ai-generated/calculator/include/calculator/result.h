#pragma once

#include <string>
#include <variant>

namespace calculator {

struct Error {
  std::string message;
};

template <typename T>
class Result {
 public:
  Result(T value) : data_{std::move(value)} {}      // NOLINT
  Result(Error error) : data_{std::move(error)} {}  // NOLINT

  bool ok() const { return std::holds_alternative<T>(data_); }
  const T& value() const { return std::get<T>(data_); }
  T Take() { return std::get<T>(std::move(data_)); }
  const Error& error() const { return std::get<Error>(data_); }

 private:
  std::variant<T, Error> data_;
};

}  // namespace calculator
