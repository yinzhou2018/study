#import "sck_capture.h"
#include <CoreGraphics/CGImage.h>
#include <CoreGraphics/CGWindow.h>
#include <Foundation/NSObjCRuntime.h>
#include <objc/NSObjCRuntime.h>

#import <Foundation/Foundation.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>

#include <iostream>

@interface CaptureDelegate : NSObject <SCStreamOutput>
- (instancetype)initWithSemaphore:(dispatch_semaphore_t)semaphore;
@property(nonatomic, weak) dispatch_semaphore_t semaphore;
@property(nonatomic, assign) CGImageRef image;
@end

@implementation CaptureDelegate
- (instancetype)initWithSemaphore:(dispatch_semaphore_t)semaphore {
  self = [super init];
  if (self) {
    _semaphore = semaphore;
    _image = nullptr;
  }
  return self;
}

- (void)stream:(SCStream*)stream didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer ofType:(SCStreamOutputType)type {
  if (type == SCStreamOutputTypeScreen && !_image) {
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    auto hr = CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    auto baseAddress = CVPixelBufferGetBaseAddress(imageBuffer);
    auto width = CVPixelBufferGetWidth(imageBuffer);
    auto height = CVPixelBufferGetHeight(imageBuffer);
    auto bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(baseAddress, width, height, 8, bytesPerRow, colorSpace,
                                                 kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
    _image = CGBitmapContextCreateImage(context);
    NSLog(@"Capture completed, stream: %@", stream);
    [stream stopCaptureWithCompletionHandler:^(NSError* error) {
      NSLog(@"Capture stopped!");
      if (error) {
        NSLog(@"Error stopping capture: %@", error);
      }
      dispatch_semaphore_signal(_semaphore);
    }];
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
  }
}

@end

CGImageRef capture(CGWindowID window_id) {
  @autoreleasepool {
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    CaptureDelegate* delegate = [[CaptureDelegate alloc] initWithSemaphore:semaphore];
    __block SCStream* strongStream = nil;  // 保障SCStream对象生存期

    auto completeHandler = ^(SCShareableContent* content, NSError* error) {
      if (error) {
        NSLog(@"Error getting shareable content: %@", error);
        dispatch_semaphore_signal(semaphore);
        return;
      }

      SCContentFilter* filter;
      SCStreamConfiguration* config = [[SCStreamConfiguration alloc] init];
      if (window_id == 0) {
        filter = [[SCContentFilter alloc] initWithDisplay:content.displays.firstObject excludingWindows:@[]];
        config.width = content.displays.firstObject.frame.size.width;
        config.height = content.displays.firstObject.frame.size.height;
      } else {
        auto idx = [content.windows indexOfObjectPassingTest:^BOOL(SCWindow* obj, NSUInteger idx, BOOL* stop) {
          return obj.windowID == window_id;
        }];
        SCWindow* window = content.windows[idx];
        filter = [[SCContentFilter alloc] initWithDesktopIndependentWindow:window];
        config.width = window.frame.size.width;
        config.height = window.frame.size.height;
      }

      config.minimumFrameInterval = CMTimeMake(1, 60);
      config.pixelFormat = kCVPixelFormatType_32BGRA;  // 正确的格式设置非常重要

      strongStream = [[SCStream alloc] initWithFilter:filter configuration:config delegate:nil];

      [strongStream addStreamOutput:delegate
                               type:SCStreamOutputTypeScreen
                 sampleHandlerQueue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)
                              error:nil];
      [strongStream startCaptureWithCompletionHandler:^(NSError* error) {
        if (error) {
          NSLog(@"Error starting capture: %@", error);
          dispatch_semaphore_signal(semaphore);
        }
      }];
    };

    [SCShareableContent getShareableContentWithCompletionHandler:completeHandler];

    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    return delegate.image;
  }
}

CGImageRef capture_with_sck(CGWindowID window_id) {
  if (@available(macOS 12.3, *)) {
    return capture(window_id);
  }
  return nullptr;
}