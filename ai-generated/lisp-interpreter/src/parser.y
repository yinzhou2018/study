%code requires {
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "ast.hpp"
}

%{
#include "ast.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// 声明解析结果
extern ValuePtr parsedResult;

// 声明词法分析器
int yylex();
void yyerror(const char* s);
%}

%union {
    double num;
    std::string* str;
    Value* val;
    std::vector<ValuePtr>* vec;
}

%token <num> NUMBER
%token <str> STRING
%token <str> SYMBOL
%token LPAREN RPAREN QUOTE

%type <val> expr atom
%type <vec> expr_list

%start program

%%

program:
    expr { parsedResult = ValuePtr($1); }
    ;

expr:
    atom
    | LPAREN expr_list RPAREN {
        $$ = new List(*$2);
        delete $2;
    }
    | QUOTE expr {
        std::vector<ValuePtr> v;
        v.push_back(std::make_shared<Symbol>("quote"));
        v.push_back(ValuePtr($2));
        $$ = new List(v);
    }
    ;

expr_list:
    /* empty */ { $$ = new std::vector<ValuePtr>(); }
    | expr_list expr { $1->push_back(ValuePtr($2)); $$ = $1; }
    ;

atom:
    NUMBER { $$ = new Number($1); }
    | STRING { $$ = new String(*$1); delete $1; }
    | SYMBOL { $$ = new Symbol(*$1); delete $1; }
    ;

%%

ValuePtr parsedResult;

void yyerror(const char* s) {
    std::cerr << "Parse error: " << s << std::endl;
}