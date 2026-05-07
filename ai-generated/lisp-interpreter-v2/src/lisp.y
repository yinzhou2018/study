% language "c++" % defines % locations % define api.namespace {lisp} % define api.parser.class {
  Parser
} % define api.value.type variant % define parse.error detailed

    %
    code requires {
#include <memory>
#include <string>
#include <vector>
#include "value.h"
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
      typedef void* yyscan_t;
#endif
    }

    % token LPAREN RPAREN QUOTE % token<double> NUMBER % token<bool> BOOLEAN % token<std::string> STRING %
    token<Symbol> SYMBOL

    % type<Value> expr atom list exprs

    % param{yyscan_t scanner} % param{std::vector<Value> & result}

    % code{
#include <sstream>
#include <stdexcept>

          namespace lisp{Value BuildList(std::vector<Value> items);
int yylex(Parser::semantic_type* yylval, Parser::location_type* yylloc, yyscan_t scanner, std::vector<Value>& result);
}
}

% %

        program : /* empty */
                  | program expr {
  result.push_back($2);
}

expr : atom | list | QUOTE expr {
  auto quoted = std::make_shared<Pair>();
  quoted->car = $2;
  quoted->cdr = Nil{};
  auto wrapper = std::make_shared<Pair>();
  wrapper->car = Symbol{"quote"};
  wrapper->cdr = quoted;
  $$ = wrapper;
}

atom : NUMBER {
  $$ = Value{$1};
}
| BOOLEAN {
  $$ = Value{$1};
}
| STRING {
  $$ = Value{$1};
}
| SYMBOL {
  $$ = Value{$1};
}

list : LPAREN RPAREN {
  $$ = Value{Nil{}};
}
| LPAREN exprs RPAREN {
  $$ = $2;
}

exprs : expr {
  $$ = BuildList({$1});
}
| expr exprs {
  auto rest = std::get<std::shared_ptr<Pair>>($2);
  auto cell = std::make_shared<Pair>();
  cell->car = $1;
  cell->cdr = rest;
  $$ = cell;
}

% %

    namespace lisp {
  Value BuildList(std::vector<Value> items) {
    Value result = Nil{};
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
      auto cell = std::make_shared<Pair>();
      cell->car = std::move(*it);
      cell->cdr = result;
      result = cell;
    }
    return result;
  }

  void Parser::error(const location_type& loc, const std::string& msg) {
    std::ostringstream oss;
    oss << "syntax error at " << loc << ": " << msg;
    throw std::runtime_error(oss.str());
  }

}  // namespace lisp
