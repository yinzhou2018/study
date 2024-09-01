#pragma once

/* 与词法分析器的接日*/
extern int yylineno;

/* 来自于词法分析器*/
void yyerror(char* s, ...);
int yylex(void);

/* 抽象语法树中的节点 */
struct ast {
  int nodetype;
  struct ast* l;
  struct ast* r;
};

struct numval {
  int nodetype; /*类型K表明常量*/
  double number;
};

/* 构造抽象语法树*/
struct ast* newast(int nodetype, struct ast* l, struct ast* r);
struct ast* newnum(double d);

/* 计算抽象语法树*/
double eval(struct ast*);

/* 删除和释放抽象语法树 */
void treefree(struct ast*);