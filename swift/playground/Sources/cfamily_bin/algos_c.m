#import "algos_c.h"
#import "../cfamily_lib/lib.h"

#import <Foundation/Foundation.h>

@interface CAlgo : NSObject
+ (int)add:(int)a b:(int)b;
@end

@implementation CAlgo
+ (int)add:(int)a b:(int)b {
  return a + b;
}
@end

int c_add(int a, int b) { 
  int c = [CAlgo add:a b:b]; 
  int d = lib_add(c, 10);
  OCLib *lib = [[OCLib alloc] init];
  int e = [lib add:c with:10];
  return d + e;
}