import Cocoa

class MainWindow: NSWindow {
  override init(
    contentRect: NSRect,
    styleMask style: NSWindow.StyleMask,
    backing backingStoreType: NSWindow.BackingStoreType,
    defer flag: Bool
  ) {
    super.init(
      contentRect: contentRect,
      styleMask: style,
      backing: backingStoreType,
      defer: flag)

    // 设置窗口背景颜色
    backgroundColor = .windowBackgroundColor

    // 创建内容视图
    let contentView = NSView(frame: contentRect)
    self.contentView = contentView

    // 创建示例标签
    let label = NSTextField(frame: NSMakeRect(20, contentRect.size.height - 40, 300, 20))
    label.stringValue = "欢迎使用Mac GUI应用！"
    label.isBezeled = false
    label.drawsBackground = false
    label.isEditable = false
    label.isSelectable = false

    contentView.addSubview(label)
  }
}
