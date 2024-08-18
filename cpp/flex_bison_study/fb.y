/* 计算器的最简版本*/
%{
#include <stdio.h>
int yylex(void);
int yyerror(char *s);
%}

/* declare tokens */
%token NUMBER
%token ADD SUB MUL DIV ABS
%token OP CP
%token EOL

%%
calclist: /* 空规则*/
| calclist exp EOL { printf(" = %d\n", $2); }
;

exp: exp ADD factor { $$ = $1 + $3; }
| exp SUB factor { $$ = $1 - $3; }
| factor {$$ = $1;}
;

factor: term {$$ = $1;}
| factor MUL term { $$ = $1 * $3; }
| factor DIV term { $$ = $1 / $3;} 
;

term: NUMBER  {$$ = $1;}
| ABS term { $$ = $2 >= 0 ? $2 : -$2; }
| ADD term { $$ = $2; }
| SUB term { $$ = -$2; }
| OP exp CP { $$ = $2; };
%%