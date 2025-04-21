#import "macos_lib.h"

@implementation MacosLib {
  int value_;
}

- (void)init:(int)a {
  value_ = a;
}

+ (int)add:(int)a b:(int)b {
  return a + b;
}

- (int)sub:(int)a {
  return self->value_ - a;
}

@end