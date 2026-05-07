#ifndef VALUE_H_
#define VALUE_H_

#include <memory>
#include <string>
#include <variant>
#include <vector>

class Environment;

struct Nil {};
struct Symbol {
  std::string name;
};
struct Pair;
struct Lambda;

using Value = std::variant<Nil, bool, double, std::string, Symbol, std::shared_ptr<Pair>, std::shared_ptr<Lambda>>;

struct Pair {
  Value car;
  Value cdr;
};

struct Lambda {
  std::vector<std::string> params;
  Value body;
  std::shared_ptr<Environment> closure;
};

std::string ValueToString(const Value& v);
bool IsTruthy(const Value& v);

#endif  // VALUE_H_
