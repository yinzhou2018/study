### 1. SwiftPM核心要点
- Targets: Target是构建的最小单元（一组源代码集合），定义了一个Module, 源码组成要么是Swift，要么是C-Based（C/C++/OC/OC++），不能两者混合, 每个Package里可以有多个Target;
- Products: Product是由一组Target构建后的操作系统支持的物理载体，**类型可以为可执行文件，静态库或动态库**，每个Package里可以包括多个Product;
- SwiftPM突出特点是支持多个语言混合Native开发，且能比较便捷的相互调用;
### 2. Swift与C-Based语言交互要点
- Swift调用C-Based语言: 由头文件生成模块映射文件适配为Swift模块导入使用;
- C-Based语言调用Swift: 由模块生成相应的头文件供include后调用接口;
- 以上两种情况都有构建工具链支持自动生成，本质问题是同样的知识在不同编程语言里有不同的语法表示;
- Swift与OC互相调用的底层原理是Swift适配OC对象内存模型并依赖OC运行时，Swift类通过从NSObject继承以及加`@objc`注解来适配，因而也可以基于运行时提供的反射机制来相互调用（全局函数不行），而不通过头文件或模块映射文件;
- Swift全局函数只能通过加`@_cdecl`注解转为C接口供C-Based调用，但参数有限制（结构体不行）;
- C-Based全局函数只能通过C接口供Swift调用;
- 实践中C/C++调用Swift基本要通过OC来适配桥接（调用Swift类）;
- Swift与外部Native语言互相调用的核心要点就是基于C-ABI或者OC对象模型及运行时;
### 3. 关于跨语言调用
- 主要需要解决的是ABI（Application Binary Interface）问题：
  - 类型映射：不同语言要能表示内存布局上相同的类型，形式上由于语言语法不同会有不同
  - 调用约定：参数传递方式，参数清理职责等
- 常用解决方案：
  - 常规Native语言间互调：基于C-ABI（边界上使用C语言类型系统及调用约定）
  - 基于统一运行时的Native语言间互调：运行时定义了类型规范，比如OC-Runtime就允许OC与Swift间能相互表示对方的类型
  - 基于特定虚拟机语言间互调：虚拟机定义类型规范，比如：JVM, CLR
  - 虚拟机语言与Native语言间互调，一般有两种思路：
    - 适配虚拟机的类型规范及调用约定，因此虚拟机会提供适配SDK，与虚拟机实现用的Native语言通信会比较便捷，与其他Navive语言会再多一层Native语言间互调问题，目前多数生产环境中使用的虚拟机都是基于C/C++实现的（JVM,V8,Python等），与C++间的互调就会相对方便
    - 遵照C-ABI,比如.net调用Native，需要虚拟机语言能表达并适配C类型
  
