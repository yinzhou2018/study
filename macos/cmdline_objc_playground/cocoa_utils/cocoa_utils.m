//
//  cocoa_utils.m
//  cocoa_utils
//
//  Created by 周寅 on 2024/11/23.
//

#import "cocoa_utils.h"

@implementation CocoaUtils

//- (id)retain {
//    return [super retain];
//}
//
//- (oneway void)release {
//    [super release];
//}
//
//- (id)autorelease {
//    return [super autorelease];
//}

-(void)dealloc {
    [super dealloc];
    NSLog(@"Cocoa utils dealloc...");
}

-(void)print {
    NSLog(@"Cocoa utils print...");
}
@end

@implementation NoParent

-(void)dealloc {
    NSLog(@"Hello");
}

@end

CocoaUtils* create(void) {
    CocoaUtils* p = [CocoaUtils new];
    NSLog(@"Retain count of obj: %ld", CFGetRetainCount((__bridge CFTypeRef)p));
    return p;
}
