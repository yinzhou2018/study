//
//  main.m
//  cmdline_objc_playground
//
//  Created by 周寅 on 2024/11/23.
//

#import <Foundation/Foundation.h>
#import "../cocoa_utils/cocoa_utils.h"

@interface Person : NSObject
-(void)print;
@end

@implementation Person

-(void)print {
    NSLog(@"Person print...");
}

-(void)dealloc {
//    [super dealloc];
    NSLog(@"Person dealloc...");
}

@end

void printImpl(Person* p) {
    [p print];
//    [p release];
}

void printPerson(void) {
    Person* p = [Person new];
//    [p autorelease];
    printImpl(p);
}

Person* createPerson(void) {
    Person* p = [Person new];
    NSLog(@"Retain count of obj: %ld", CFGetRetainCount((__bridge CFTypeRef)p));
    return p;
}

/* 几点结论：
 * 1. mac下动态库的运行时加载路径由动态库编译时install path设置决定，除非是一些通用基础库设置到系统公共库目录（比如：usr/lib），
 * 一般设置为以`@rpath`为根的相对目录，同时设置run path为`@executable_path`或`@loader_path`为根的相对目录，方便动态库统一放在可执行文件安装目录内，
 * 并能支持运行时正确加载。这一点与linux有所差别，linux不需要动态库自身编译做install path相关设置，只需链接该动态库的可执行程序构建时设置好run path；
 * 2. mrc(manual reference counting)模式下，通常用autoreleasepool机制来实现非局部对象（比如一个函数里创建对象并返回）的自动释放(调用autorelease）
 * ，否则程序员就需要在不再需要对象的地方显示调用release，门槛高，容易出现泄漏或重复释放的问题，但副作用就是对象释放可能偏晚；
 * 3. arc(automatic reference counting)模式下，不需要autoreleasepool机制；
 * 4. arc+mrc混合模式下（实际开发的常态，因为有遗留代码，比如cocoa框架），跨模块（静态库或动态库或框架）边界涉及接口调用（比如调用cocoa框架）返回对象时，
 * 仍然需要使用autoreleasepool，否则可能有内存泄漏发生，具体分析几种情况行为：
 * 4.1. 被调方是mrc模式，这种情况如第2点所说一般需要显式调用autorelease，调用方不管哪种模式都需要把返回值包装在autoreleasepool内，调用方为mrc模式时，
 * 对象随autoreleasepool销毁而释放，调用方为arc模式时，返回值会立即被丢给编译器插入的objc_retainAutoReleasedReturnValue运行时接口，里面会对
 * 返回对象调用objc_retain增加引用计数到2，如果不包装在autoreleasepool基于销毁时自动减小引用计数则会有泄漏；
 * 4.2. 被调方是arc模式，编译器会在返回前插入objc_autoreleaseReturnValue接口调用，接口里判断调用方模式，如果也是arc模式，则设置一个标志并直接返回对象
 * ，如果是mrc模式，则插入一个autorelease调用，调用方为mrc模式时，基于autoreleasepool机制释放对象，调用方为arc模式时，返回值会立即被丢给编译器插入的
 * objc_retainAutoReleasedReturnValue运行时接口，里面基于被调时arc模式返回前设置的标志，不再需要调用objc_retain，保持对象的引用计数为1，在接收对象
 * 的变量超过生命周期时自动调用release释放对象；
 * 5. objc_autoreleaseReturnValue/objc_retainAutoReleasedReturnValue是引入arc模式后，兼容各种模式互相调用内存不出问题，同时针对纯arc模式没有
 * 多余的retain/release提升性能的核心机制，具体细节参见：
 * https://www.galloway.me.uk/2012/02/how-does-objc_retainautoreleasedreturnvalue-work/
 */
 
int main(int argc, const char * argv[]) {
   @autoreleasepool {
//        printPerson();
        NSLog(@"Hello, World!");
        CocoaUtils* p = create();
//        Person* pp = createPerson();
//        [pp print];
//        [p autorelease];
        [p print];
        NSLog(@"Retain count of obj: %ld", CFGetRetainCount((__bridge CFTypeRef)p));
//        [p release];
    }
    
    //    printPerson();
    NSLog(@"End");
    
    return 0;
}
