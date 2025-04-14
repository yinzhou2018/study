#ifndef _LIB_H_
#define _LIB_H_

// 定义一个 C 结构体
typedef struct {
  int x;
  int y;
} Point;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

Point makePoint(int x, int y);
void printMessage(const char* message);
int add(int a, int b);

// 声明 Go 函数，这些函数将在 C 中被调用
extern void goCallback(int);
extern int sumArray(int*, int);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _LIB_H_