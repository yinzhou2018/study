## 基本结论
- 截取整个屏幕DXGI方式最快
- 硬件加速窗口GDI方式无法截取
- PrintWindow方式无法截取整个屏幕
- 普通窗口采用GDI截取最快
- 硬件加速窗口DXGI方式快于PrintWindow方式
- 普通窗口截图耗时截图:

  ![普通窗口](./normal.png)
- 硬件加速窗口截图耗时截图：

  ![硬件加速窗口](./accelerated.png)