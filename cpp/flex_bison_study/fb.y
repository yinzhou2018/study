/* 计算器的最简版本2.0：基于bison内置的优先级规则能力，简化语法规则书写，解析也更快*/
%{
#include <stdio.h>
#include <stdlib.h>
#include "../fb.h"
%}

%union {
  struct ast *a; 
  double d;
  struct symbol *s;
  struct symlist *sl;
  int fn;
}

%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token EOL
%token IF THEN ELSE WHILE DO LET

%type <a> exp stmt list explist
%type <sl> symlist

%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS UPLUS

%start calclist

%%
calclist: /* 空规则*/
| calclist exp EOL {
    printf(" = %4.4g\n", eval($2));
    treefree($2);
  }
| calclist LET NAME '(' symlist ')' '=' list EOL {
    dodef($3, $5, $8);
    printf("Defined %s\n", $3->name);
  }
| calclist error EOL { yyerrok; printf("> "); }
;

exp: exp '+' exp { $$ = newast('+', $1, $3); }
| exp '-' exp { $$ = newast('-', $1, $3); }
| exp '*' exp { $$ = newast('*', $1, $3); }
| exp '/' exp { $$ = newast('/', $1, $3); } 
| '|' exp { $$ = newast('|', $2, NULL); }
| '+' exp %prec UPLUS{ $$ = newast('P', $2, NULL); }
| '-' exp %prec UMINUS{ $$ = newast('M', $2, NULL); }
| '(' exp ')' { $$ = $2; };
| NUMBER { $$ = newnum($1); }
| exp CMP exp { $$ = newcmp($2, $1, $3); }
| NAME '=' exp { $$ = newasgn($1, $3); }
| NAME { $$ = newref($1); }
| FUNC '(' explist ')' { $$ = newfunc($1, $3); }
| NAME '(' explist ')' { $$ = newcall($1, $3); }
;

explist: exp
| exp ',' explist { $$ = newast('L', $1, $3); }
;

symlist: NAME { $$ = newsymlist($1, NULL); }
| NAME ',' symlist { $$ = newsymlist($1, $3); }
;

stmt: IF exp THEN list { $$ = newflow('I', $2, $4, NULL); }
| IF exp THEN list ELSE list { $$ = newflow('I', $2, $4, $6); }
| WHILE exp DO list { $$ = newflow('W', $2, $4, NULL); }
| exp
;

list: { $$ = NULL; }
| stmt ';' list {
    if ($3 == NULL) 
      $$ = $1;
    else
      $$ = newast('L', $1, $3);
  }
;

%%