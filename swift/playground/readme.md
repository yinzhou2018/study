### 1. SwiftPM核心要点
- Targets: Target是构建的最小单元（一组源代码集合），定义了一个Module, 源码组成要么是Swift，要么是C-Based（C/C++/OC/OC++），不能两者混合, 每个Package里可以有多个Target;
- Products: Product是由一组Target构建后的操作系统支持的物理载体，**类型可以为可执行文件，静态库或动态库**，每个Package里可以包括多个Product;
- SwiftPM突出特点是支持多个语言混合Native开发，且能比较便捷的相互调用;
### 2. Swift与C-Based语言交互要点
- Swift调用C-Based语言: 由头文件生成模块映射文件适配为Swift模块导入使用;
- C-Based语言调用Swift: 由模块生成相应的头文件供include后调用接口;
- 以上两种情况都有构建工具链支持自动生成，本质问题是同样的知识在不同编程语言里有不同的语法表示;
- Swift与OC共享相同的运行时，可以基于运行时提供的反射机制来相互调用（全局函数不行），而不通过头文件或模块映射文件;
- Swift类只能给OC调用，且必须满足条件：从NSObject继承 + 加@objc注解;
- Swift全局函数只能通过加@_cdecl注解转为C接口供C-Based调用，但参数有限制（结构体不行）;
- C-Based全局函数只能通过C接口供Swift调用;
- 实践中C/C++调用Swift基本要通过OC来适配桥接（调用Swift类）;
### 3. 跨语言调用的主要问题
- 
