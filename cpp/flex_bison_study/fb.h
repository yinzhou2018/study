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

struct symbol {
  char *name;
  double value;
  struct ast *func; 
  struct symlist *syms;
};

#define NHASH 9997
struct symbol symtab[NHASH];
struct symbol *lookup(char*);

struct symlist {
  struct symbol *sym;
  struct symlist *next;
};

enum bifs {
  B_sqrt = 1,
  B_exp,
  B_log,
  B_print
};

struct fncall {
  int nodetype; 
  struct ast *l;
  enum bifs functype;
};

struct ufncall {
  int nodetype;
  struct ast *l; 
  struct symbol *s;
};

struct flow {
  int nodetype;
  struct ast *cond;
  struct ast *tl; /* then 分支或者do 语句 */
  struct ast *el; /*可选的else分支*/
};

struct symref {
  int nodetype;
  struct symbol *s;
};

struct symasgn {
  int nodetype;
  struct symbol *s;
  struct ast *v; 
};

struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);

/* 构造抽象语法树 */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newnum(double d);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);

/* 定义函数*/
void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);


/* 计算抽象语法树*/
double eval(struct ast*);

/* 删除和释放抽象语法树 */
void treefree(struct ast*);