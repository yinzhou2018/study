// swift-tools-version:5.5
import PackageDescription

let package = Package(
  name: "MacGUIApp",
  platforms: [
    .macOS(.v11)
  ],
  dependencies: [
    // 这里可以添加Swift包依赖
    // 例如: .package(url: "https://github.com/example/package.git", from: "1.0.0"),
  ],
  targets: [
    .executableTarget(
      name: "MacGUIApp",
      dependencies: [],
      path: "src"
    )
  ]
)
