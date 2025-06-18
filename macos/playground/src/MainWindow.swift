import Cocoa

// 结论：
// 1. layer-hosted view：只能基于layer来绘制内容，view的draw方法不会调用，需要刷新绘制时只能调用layer.setNeedDisplay方法；
// 2. layer-backed view：可以基于layer和view来绘制内容，默认调用view的draw方法,如果view实现CALayerDelegate协议，则会自动调用协议的draw方法；
// 3. 当layer-backed view的wantsUpdateLayer属性为true时，draw方法不会调用，而是调用updateLayer，需要刷新layer绘制时需要调用layer.setNeedDisplay方法；

class LayerHostedView: NSView, CALayerDelegate {
  override init(frame frameRect: NSRect) {
    super.init(frame: frameRect)
    layer = CALayer()
    layer?.backgroundColor = NSColor.systemBlue.cgColor
    layer?.cornerRadius = 8.0
    layer?.delegate = self
    wantsLayer = true
  }

  required init?(coder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }

  func draw(_ layer: CALayer, in ctx: CGContext) {
    print("LayerHostedView::draw...")

    let colors: [NSColor] = [
      .systemBlue, .systemRed, .systemGreen, .systemYellow, .systemOrange, .systemPurple,
    ]
    let randomColor = colors.randomElement() ?? .black
    ctx.setFillColor(randomColor.cgColor)
    ctx.fill(layer.bounds)
  }
}

class LayerBackedView: NSView {
  override var wantsUpdateLayer: Bool {
    return true
  }

  override init(frame frameRect: NSRect) {
    super.init(frame: frameRect)
    wantsLayer = true
    layer?.backgroundColor = NSColor.systemRed.cgColor
  }

  required init?(coder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }

  override func updateLayer() {
    print("LayerBackedView::updateLayer...")
    let colors: [NSColor] = [
      .systemBlue, .systemRed, .systemGreen, .systemYellow, .systemOrange, .systemPurple,
    ]
    let randomColor = colors.randomElement() ?? .black
    layer?.backgroundColor = randomColor.cgColor
  }

  // 可以自定义layer-backed view使用特定的layer类型
  // override func makeBackingLayer() -> CALayer {
  //   let layer = CALayer()
  //   return layer
  // }
}

class NonLayerBackedView: NSView {

  override init(frame frameRect: NSRect) {
    super.init(frame: frameRect)
    // 监听frame变化
    NotificationCenter.default.addObserver(
      self,
      selector: #selector(frameDidChange(_:)),
      name: NSView.frameDidChangeNotification,
      object: self)
  }

  required init?(coder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }

  deinit {
    NotificationCenter.default.removeObserver(
      self,
      name: NSView.frameDidChangeNotification,
      object: self)
  }

  @objc func frameDidChange(_ notification: Notification) {
    print("NonLayerBackedView::frameDidChange...")
    for subview in subviews {
      if let layerHostedView = subview as? LayerHostedView {
        layerHostedView.layer?.setNeedsDisplay()
      } else if let layerBackedView = subview as? LayerBackedView {
        layerBackedView.setNeedsDisplay(layerBackedView.bounds)
      }
    }
  }

  override func draw(_ dirtyRect: NSRect) {
    // print("[NonLayerBackedView]view layer: \(layer)")
    let ctx = (NSGraphicsContext.current?.cgContext)!
    drawContent(in: ctx)
  }

  // func draw(_ layer: CALayer, in ctx: CGContext) {
  //   // print("[layerdelegate]draw layer: \(layer)")
  //   drawContent(in: ctx)
  // }

  private func drawContent(in ctx: CGContext) {
    // 要绘制的文本
    let text = "Hello, Mac GUI App!"
    // 文本字体
    let font = CTFontCreateWithName("Helvetica-Bold" as CFString, 32, nil)
    // 文本属性
    let attributes: [NSAttributedString.Key: Any] = [
      .font: font,
      .foregroundColor: NSColor.black.cgColor,
    ]
    // 创建属性字符串
    let attributedString = NSAttributedString(string: text, attributes: attributes)
    // 创建 CTLine 对象
    let line = CTLineCreateWithAttributedString(attributedString)

    // 保存当前图形状态
    ctx.saveGState()
    // 转换坐标系，将原点从左上角移动到左下角
    ctx.translateBy(x: 20, y: 50)
    // 移除 Y 轴翻转操作

    // 绘制文本
    CTLineDraw(line, ctx)

    // 恢复之前保存的图形状态
    ctx.restoreGState()
  }
}

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

    backgroundColor = .windowBackgroundColor

    print("original content view: \(self.contentView)")
    print("original content view layer: \(self.contentView?.layer)")

    let nonLayerBackedView = NonLayerBackedView(frame: contentRect)
    nonLayerBackedView.layer?.backgroundColor = NSColor.systemGreen.cgColor
    let hostedView = LayerHostedView(frame: NSRect(x: 20, y: 20, width: 100, height: 100))
    nonLayerBackedView.addSubview(hostedView)
    let backedView = LayerBackedView(frame: NSRect(x: 150, y: 20, width: 100, height: 100))
    nonLayerBackedView.addSubview(backedView)
    self.contentView = nonLayerBackedView
  }
}
