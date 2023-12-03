#include <stdio.h>

// 声明Rust函数
extern int add(int a, int b);

int main() {
    printf("The sum is: %d\n", add(2, 3));
    return 0;
}