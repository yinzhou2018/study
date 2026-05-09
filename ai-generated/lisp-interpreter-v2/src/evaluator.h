#ifndef EVALUATOR_H_
#define EVALUATOR_H_

#include <memory>
#include <string>
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
  Value ApplySpecialForm(const Symbol& name, const Value& args, std::shared_ptr<Environment> env);

  Value EvalDefine(const Value& args, std::shared_ptr<Environment> env);
  Value EvalIf(const Value& args, std::shared_ptr<Environment> env);
  Value EvalCond(const Value& args, std::shared_ptr<Environment> env);
  Value EvalLet(const Value& args, std::shared_ptr<Environment> env);
  Value EvalLambda(const Value& args, std::shared_ptr<Environment> env);

  std::shared_ptr<Environment> global_env_;
};

#endif  // EVALUATOR_H_
