#pragma once

#include "ast.h"
#include "result.h"

namespace calculator {

namespace evaluator {

Result<double> Evaluate(const Expr& expr);

}  // namespace evaluator

}  // namespace calculator
