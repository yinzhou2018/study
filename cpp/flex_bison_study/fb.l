%option noyywrap nodefault yylineno

%{
#include "fb.tab.h"
#include "../fb.h"
%}

/* 浮点数指数部分 */
EXP ([Ee][-+]?[0-9]+)

%%
"+" { return yytext[0]; }
"-" { return  yytext[0]; }
"*" { return  yytext[0]; }
"/" { return  yytext[0]; }
"|" { return  yytext[0]; }
"(" { return  yytext[0]; }
")" { return  yytext[0]; }
[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}? { yylval.d = atof(yytext); return NUMBER; }
[\n] { return EOL; }
[ \t] { /* 忽略空白字符*/ }
. { yyerror("Mystery character %c\n", *yytext); }
%%