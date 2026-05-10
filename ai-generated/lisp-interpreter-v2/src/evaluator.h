#ifndef EVALUATOR_H_
#define EVALUATOR_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "environment.h"
#include "value.h"

class Evaluator {
 public:
  Evaluator();

  Value Eval(const Value& expr);

 private:
  Value EvalExpr(const Value& expr, std::shared_ptr<Environment> env);
  Value EvalList(const Value& exprs, std::shared_ptr<Environment> env);
  Value ApplyFunction(const Value& func, const Value& args);

  Value EvalDefine(const Value& args, std::shared_ptr<Environment> env);
  Value EvalIf(const Value& args, std::shared_ptr<Environment> env);
  Value EvalCond(const Value& args, std::shared_ptr<Environment> env);
  Value EvalLet(const Value& args, std::shared_ptr<Environment> env);
  Value EvalLambda(const Value& args, std::shared_ptr<Environment> env);
  Value EvalAnd(const Value& args, std::shared_ptr<Environment> env);
  Value EvalOr(const Value& args, std::shared_ptr<Environment> env);
  Value EvalQuote(const Value& args, std::shared_ptr<Environment> env);
  Value EvalSequence(const Value& exprs, std::shared_ptr<Environment> env);

  using SpecialFormHandler = Value (Evaluator::*)(const Value&, std::shared_ptr<Environment>);
  std::unordered_map<std::string, SpecialFormHandler> special_forms_;
  std::shared_ptr<Environment> global_env_;
};

#endif  // EVALUATOR_H_
