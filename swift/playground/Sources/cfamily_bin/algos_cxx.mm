#import "algos_cxx.h"
#import "../../.build/debug/swift_lib.build/swift_lib-Swift.h"
#import "../cfamily_lib/lib.h"
#import <Foundation/Foundation.h>
#import <iostream>

@interface CXXAlgo : NSObject
+ (int)add:(int)a to:(int)b;
@end

@implementation CXXAlgo
+ (int)add:(int)a to:(int)b {
  return lib_add_v2(a, b);
}
@end

int cxx_add(int a, int b) {
  int left = [CXXAlgo add:a to:b];
  SwiftClass *swiftClass1 = [[SwiftClass alloc] initWithValue:10];
  SwiftClass *swiftClass2 = [[SwiftClass alloc] initWithValue:left];
  int right = [swiftClass1 add:swiftClass2];
  return swift_add(right, 20);
}