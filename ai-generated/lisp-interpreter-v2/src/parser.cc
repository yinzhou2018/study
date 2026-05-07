#include "parser.h"

#include "lisp.tab.h"

#include <stdexcept>

struct yy_buffer_state;
using yyscan_t = void*;

// Flex reentrant scanner functions (compiled as C++)
int yylex_init(yyscan_t* scanner);
int yylex_destroy(yyscan_t scanner);
yy_buffer_state* yy_scan_string(const char* str, yyscan_t scanner);
void yy_delete_buffer(yy_buffer_state* buffer, yyscan_t scanner);
int yylex(lisp::Parser::semantic_type* yylval, lisp::Parser::location_type* yylloc, yyscan_t scanner);

namespace lisp {
int yylex(Parser::semantic_type* yylval,
          Parser::location_type* yylloc,
          yyscan_t scanner,
          std::vector<Value>& /*result*/) {
  return ::yylex(yylval, yylloc, scanner);
}
}  // namespace lisp

std::vector<Value> Parse(const std::string& source) {
  yyscan_t scanner;
  yylex_init(&scanner);
  auto buf = yy_scan_string(source.c_str(), scanner);
  std::vector<Value> result;
  lisp::Parser parser(scanner, result);
  parser.parse();
  yy_delete_buffer(buf, scanner);
  yylex_destroy(scanner);
  return result;
}
