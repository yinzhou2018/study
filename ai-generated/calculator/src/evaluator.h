#pragma once

#include "calculator/ast.h"
#include "calculator/result.h"

namespace calculator {

namespace evaluator {

Result<double> Evaluate(const Expr& expr);

}  // namespace evaluator

}  // namespace calculator
