#import "lib.h"

int lib_add(int a, int b) { return a + b; }

int lib_add_v2(int a, int b) { return a + b; }

LibData lib_add_v1(LibData a, LibData b) {
  LibData result;
  result.a = a.a + b.a;
  result.b = a.b + b.b;
  return result;
}

struct LibData lib_add_v3(struct LibData* a, struct LibData* b) {
  return LibData{a->a + b->a, a->b + b->b};
}

@implementation OCLib
- (int)add:(int)a with:(int)b {
  return a + b;
}
@end