#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <vector>

#include "value.h"

std::vector<Value> Parse(const std::string& source);

#endif  // PARSER_H_
