#include "value.h"

#include <sstream>

std::string ValueToString(const Value& v) {
  return std::visit(
      [](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Nil>) {
          return "()";
        } else if constexpr (std::is_same_v<T, bool>) {
          return arg ? "#t" : "#f";
        } else if constexpr (std::is_same_v<T, double>) {
          if (arg == static_cast<int64_t>(arg)) {
            return std::to_string(static_cast<int64_t>(arg));
          }
          return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
          return "\"" + arg + "\"";
        } else if constexpr (std::is_same_v<T, Symbol>) {
          return arg.name;
        } else if constexpr (std::is_same_v<T, std::shared_ptr<Pair>>) {
          std::ostringstream oss;
          oss << "(";
          bool first = true;
          Value cur = arg;
          while (auto* p = std::get_if<std::shared_ptr<Pair>>(&cur)) {
            if (!first)
              oss << " ";
            first = false;
            oss << ValueToString((*p)->car);
            cur = (*p)->cdr;
          }
          if (!std::holds_alternative<Nil>(cur)) {
            oss << " . " << ValueToString(cur);
          }
          oss << ")";
          return oss.str();
        } else if constexpr (std::is_same_v<T, std::shared_ptr<Lambda>>) {
          return "<lambda>";
        }
        return "<unknown>";
      },
      v);
}

bool IsTruthy(const Value& v) {
  if (auto* b = std::get_if<bool>(&v))
    return *b;
  if (std::holds_alternative<Nil>(v))
    return false;
  return true;
}
