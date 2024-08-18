#include <stdio.h>

int yyparse(void);
int main(int argc, char **argv)
{
  yyparse();
  return 0;
}

int yyerror(char *s)
{
  fprintf(stderr, "error: %s\n", s);
  return 0;
}