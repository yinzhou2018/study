import Cocoa

@objc public class AppDelegate: NSObject, NSApplicationDelegate {
  @objc public var window: NSWindow?

  @objc public override init() {
    super.init()
  }

  @objc public func applicationDidFinishLaunching(_ notification: Notification) {
    // 创建主窗口
    let frame = NSRect(x: 100, y: 100, width: 800, height: 600)
    let style: NSWindow.StyleMask = [.titled, .closable, .miniaturizable, .resizable]

    window = MainWindow(
      contentRect: frame,
      styleMask: style,
      backing: .buffered,
      defer: false)

    window?.title = "Mac GUI App"
    window?.center()

    // 显示窗口并使其成为主窗口
    window?.makeKeyAndOrderFront(nil)

    // 激活应用程序
    NSApp.activate(ignoringOtherApps: true)
  }

  @objc public func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool
  {
    return true
  }
}
