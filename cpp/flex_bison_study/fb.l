%option noyywrap
%{
#include "fb.tab.h"
%}

%%
"+" {return ADD; }
"-" {return SUB; }
"*" {return MUL; }
"/" { return DIV; }
"|" { return ABS; }
"(" { return OP; }
")" { return CP; }
[0-9]+ {yylval = atoi(yytext); return NUMBER; }
0x[0-9a-fA-F]+ {yylval = strtol(yytext + 2, NULL, 16); return NUMBER; }
[\n] { return EOL; }
[ \t] {/*忽略空白字符*/ }
"//".* {/*忽略注释*/ }
. { printf("Mystery character %c\n", *yytext); }
%%