% {
#include <string>
#include "lisp.tab.h"

#define YYSTYPE lisp::Parser::semantic_type
#define YYLTYPE lisp::Parser::location_type

  // Bring token constants into scope
  static constexpr int LPAREN_ = lisp::Parser::token::LPAREN;
  static constexpr int RPAREN_ = lisp::Parser::token::RPAREN;
  static constexpr int QUOTE_ = lisp::Parser::token::QUOTE;
  static constexpr int NUMBER_ = lisp::Parser::token::NUMBER;
  static constexpr int BOOLEAN_ = lisp::Parser::token::BOOLEAN;
  static constexpr int STRING_ = lisp::Parser::token::STRING;
  static constexpr int SYMBOL_ = lisp::Parser::token::SYMBOL;
  %
}

% option reentrant bison - bridge bison -
    locations % option noinput nounput noyywrap

        % %

        [ \t\r] +
    {/* skip whitespace */};
[^\n] * { /* skip line comments */ }
\n {
  yylloc->lines(yyleng);
  yylloc->step();
}
"(" {
  return LPAREN_;
}
")" {
  return RPAREN_;
}
"'" {
  return QUOTE_;
}
#t{yylval->emplace < bool>(true); return BOOLEAN_; }
#f{yylval->emplace < bool>(false); return BOOLEAN_; }
[0 - 9] + (\.[0 - 9] +) ? {
  yylval->emplace<double>(std::stod(yytext));
  return NUMBER_;
}
\"([^"\\]|\\.)*\"   { yylval->emplace<std::string>(std::string(yytext + 1, yyleng - 2)); return STRING_; }
[a-zA-Z_!$%&*+\-./:<=>?@^~][a-zA-Z0-9_!$%&*+\-./:<=>?@^~]* {
  yylval->emplace<Symbol>(Symbol{yytext});
  return SYMBOL_; }
.                   {
  throw std::runtime_error(std::string("unexpected character: ") + yytext);
}

% %
