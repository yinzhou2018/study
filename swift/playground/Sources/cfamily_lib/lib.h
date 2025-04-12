#ifndef _LIB_H_
#define _LIB_H_

struct LibData {
  int a;
  int b;
};

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

int lib_add(int a, int b);
struct LibData lib_add_v1(struct LibData a, struct LibData b);
struct LibData lib_add_v3(struct LibData* a, struct LibData* b);

#ifdef __cplusplus
}
#endif  // __cplusplus

int lib_add_v2(int a, int b);

#ifdef __OBJC__
#import <Foundation/Foundation.h>
@interface OCLib: NSObject
- (int)add:(int)a with:(int)b;
@end
#endif  // __OBJC__

#endif  // _LIB_H_