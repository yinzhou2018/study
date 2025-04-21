#import "SwiftBridge.h"
#import <Cocoa/Cocoa.h>

// 保持全局引用以防止被释放
static id<NSApplicationDelegate> appDelegate = nil;

void initializeApplication(void) {
  [NSApplication sharedApplication];
  Class delegateClass = NSClassFromString(@"MacGUIApp.AppDelegate");
  if (delegateClass) {
    appDelegate = [[delegateClass alloc] init];
    [NSApp setDelegate:appDelegate];
  }
}

void runMainEventLoop(void) { [NSApp run]; }