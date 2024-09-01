#include <iostream>

extern "C" {

int yyparse(void);

}  // extern "C"

int main(int argc, char** argv) {
  std::cout << "> ";
  yyparse();
  return 0;
}
