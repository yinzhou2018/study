# 基础计算器 Web 应用

基于 React + TypeScript + Vite 构建的基础四则运算计算器。

## 技术栈

- React 18 + TypeScript + Vite
- useReducer 状态机管理
- Vitest 单元测试

## 核心设计

- **链式求值**：按输入顺序计算，无运算符优先级（`1 + 2 × 3 = 9`）
- **精度处理**：`roundResult` 四舍五入截断至 10 位小数，消除浮点误差
- **除零处理**：除零显示"错误"，仅 AC 可恢复
- **状态机**：Idle → EnteringOperand → OperatorSelected → Result / Error

## 目录结构

```
src/
├── main.tsx                  # 入口
├── App.tsx                   # 应用根组件（含键盘支持）
├── App.css / index.css       # 样式
├── components/
│   ├── Calculator.tsx        # 计算器容器
│   ├── Display.tsx           # 显示屏
│   ├── Button.tsx            # 按钮组件
│   └── ButtonPad.tsx         # 按钮网格
├── hooks/
│   └── useCalculator.ts      # 封装 reducer 的 Hook
└── lib/
    ├── roundResult.ts        # 浮点精度处理
    ├── compute.ts            # 四则运算 + 除零拦截
    ├── formatDisplay.ts      # 显示格式化（溢出/科学计数法）
    └── calculator.ts         # reducer 状态机
```

## 运行

```bash
npm install
npm run dev        # 开发服务器 http://localhost:5173
npm run build      # 生产构建
npm run preview    # 预览构建产物
npm run test       # 运行单元测试
```

## 功能

### P0 核心功能
- 数字输入 0-9、小数点
- 四则运算（+ − × ÷）
- 等号、AC 清除
- 显示屏、响应式布局
- 除零错误处理

### P1 重要功能
- 连续运算（基于结果继续）
- 退格删除
- 正负号切换（+/−）
- 键盘输入支持（0-9, +−×/, Enter, Escape, Backspace）

## 测试

42 个单元测试覆盖：核心纯函数（roundResult / compute / formatDisplay）、状态机全部转换路径、验收用例（链式求值、除零、边界情况）。
