%{
#include "parser.hpp"
#include <iostream>
#include <string>
%}

%option noyywrap
%option nodefault
%option yylineno

%%

[ \t\r\n]+          ; // 忽略空白字符
;[^\n]*             ; // 忽略注释

"("                 { return LPAREN; }
")"                 { return RPAREN; }
"'"                 { return QUOTE; }

\"([^\"\\]|\\.)*\"  {
    std::string s(yytext);
    yylval.str = new std::string(s.substr(1, s.length() - 2));
    return STRING;
}

-?[0-9]+(\.[0-9]+)? {
    yylval.num = std::stod(yytext);
    return NUMBER;
}

[a-zA-Z_+\-*/<>=!?][a-zA-Z0-9_+\-*/<>=!?]* {
    yylval.str = new std::string(yytext);
    return SYMBOL;
}

.                   {
    std::cerr << "Unexpected character: " << yytext << " at line " << yylineno << std::endl;
}

%%