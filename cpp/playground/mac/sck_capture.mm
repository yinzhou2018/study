#import "sck_capture.h"

#import <Foundation/Foundation.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>

#include <iostream>

@interface CaptureDelegate : NSObject <SCStreamOutput>
@property(nonatomic, copy) void (^frameHandler)(CGImageRef);
@end

@implementation CaptureDelegate

- (void)stream:(SCStream*)stream didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer ofType:(SCStreamOutputType)type {
  if (type == SCStreamOutputTypeScreen) {
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(imageBuffer, 0);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context =
        CGBitmapContextCreate(CVPixelBufferGetBaseAddress(imageBuffer), CVPixelBufferGetWidth(imageBuffer),
                              CVPixelBufferGetHeight(imageBuffer), 8, CVPixelBufferGetBytesPerRow(imageBuffer),
                              colorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);

    CGImageRef cgImage = CGBitmapContextCreateImage(context);
    if (self.frameHandler) {
      self.frameHandler(cgImage);
    }

    CGImageRelease(cgImage);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
  }
}

@end

void captureDisplay(bool includeWindows) {
  @autoreleasepool {
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

    [SCShareableContent getShareableContentWithCompletionHandler:^(SCShareableContent* content, NSError* error) {
      if (error) {
        NSLog(@"Error getting shareable content: %@", error);
        return;
      }

      SCContentFilter* filter;
      if (includeWindows) {
        filter = [[SCContentFilter alloc] initWithDisplay:content.displays.firstObject excludingWindows:@[]];
      } else {
        filter = [[SCContentFilter alloc] initWithDisplay:content.displays.firstObject
                                         excludingWindows:content.windows];
      }

      // 创建流配置
      SCStreamConfiguration* config = [[SCStreamConfiguration alloc] init];
      config.width = 1920;
      config.height = 1080;
      config.minimumFrameInterval = CMTimeMake(1, 30);
      config.queueDepth = 5;

      // 创建捕获流
      NSError* streamError = nil;
      SCStream* stream = [[SCStream alloc] initWithFilter:filter configuration:config delegate:nil];

      if (streamError) {
        NSLog(@"Error creating stream: %@", streamError);
        return;
      }

      // 创建代理处理捕获的帧
      CaptureDelegate* delegate = [[CaptureDelegate alloc] init];
      delegate.frameHandler = ^(CGImageRef image) {
        // 在这里处理捕获的帧
        NSString* path = @"screen_with_sck.png";
        NSURL* url = [NSURL fileURLWithPath:path];
        CGImageDestinationRef destination =
            CGImageDestinationCreateWithURL((__bridge CFURLRef)url, kUTTypePNG, 1, NULL);
        CGImageDestinationAddImage(destination, image, nil);
        CGImageDestinationFinalize(destination);
        CFRelease(destination);

        // 停止捕获
        [stream stopCaptureWithCompletionHandler:^(NSError* error) {
          if (error) {
            NSLog(@"Error stopping capture: %@", error);
          }
          dispatch_semaphore_signal(semaphore);
        }];
      };

      // 添加流输出
      [stream addStreamOutput:delegate
                         type:SCStreamOutputTypeScreen
           sampleHandlerQueue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)
                        error:nil];

      // 开始捕获
      [stream startCaptureWithCompletionHandler:^(NSError* error) {
        if (error) {
          NSLog(@"Error starting capture: %@", error);
          dispatch_semaphore_signal(semaphore);
        }
      }];
    }];

    // 等待捕获完成
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
  }
}

void capture_with_sck() {
  if (@available(macOS 12.3, *)) {
    captureDisplay(true);
  }
}