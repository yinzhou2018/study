#import <iostream>

#import <knative.h>

int main() {
  std::cout << "Hello World!" << std::endl;

  [KnativeKAppleLibKt doInitAppleLib];  
  auto clazz_p = [KnativeClazz new];
  auto p = [clazz_p memberP:10];  
  auto result = [p unsignedLongLongValue];
  std::cout << "p = " << result << std::endl;
  [clazz_p iMember];
  auto dclazz_p = [[KnativeDataClass alloc] initWithField:@"Hello"];
  auto dresult = [dclazz_p field];
  std::cout << "dclazz_p field: " << [dresult UTF8String] << std::endl;
  auto dresult2 = [[KnativeObject shared] field];
  std::cout << "dclazz_p field: " << [dresult2 UTF8String] << std::endl;
  [KnativeKLibKt printDataClassData:dclazz_p];
  auto block = ^(NSString* str) {
    return str;
  };
  auto block_result = [KnativeKLibKt acceptFunF:block];
  auto block_result2 = [KnativeKLibKt acceptFunF:[KnativeKLibKt supplyFun]];
  std::cout << "block_result: " << [block_result UTF8String] << std::endl;
  std::cout << "block_result2: " << [block_result2 UTF8String] << std::endl;
  
  return 0;
}