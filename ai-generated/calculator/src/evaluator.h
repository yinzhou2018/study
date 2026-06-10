#pragma once

#include <string>

#include "ast.h"
#include "result.h"

namespace calculator {

namespace evaluator {

Result<double> Evaluate(const Expr& expr);

Result<double> EvaluateExpression(const std::string& input);

}  // namespace evaluator

}  // namespace calculator
