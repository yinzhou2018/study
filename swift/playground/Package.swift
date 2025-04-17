// swift-tools-version: 5.10
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
  name: "playground",
  products: [
    .executable(name: "cfamily_bin", targets: ["cfamily_bin"]),
    .executable(name: "swift_bin", targets: ["swift_bin"]),
    .library(name: "swift_lib", type: .dynamic, targets: ["swift_lib"]),
    .library(name: "cfamily_lib", type: .dynamic, targets: ["cfamily_lib"]),
  ],
  targets: [
    .executableTarget(
      name: "swift_bin",
      dependencies: ["swift_lib", "cfamily_lib"]
    ),
    .executableTarget(
      name: "cfamily_bin",
      dependencies: ["swift_lib", "cfamily_lib"],
      cSettings: [
        .define("OBJC_OLD_DISPATCH_PROTOTYPES", to: "0")
      ],
      cxxSettings: [
        .define("OBJC_OLD_DISPATCH_PROTOTYPES", to: "0"),
        .headerSearchPath("."),
        .unsafeFlags(["-std=c++20"]),
        .unsafeFlags(["-x", "objective-c++"]),
      ]
    ),
    .target(
      name: "swift_lib"
    ),
    .target(
      name: "cfamily_lib",
      publicHeadersPath: "./",
      cSettings: [
        .define("OBJC_OLD_DISPATCH_PROTOTYPES", to: "0")
      ],
      cxxSettings: [
        .define("OBJC_OLD_DISPATCH_PROTOTYPES", to: "0"),
        .headerSearchPath("."),
        .unsafeFlags(["-std=c++20"]),
        .unsafeFlags(["-x", "objective-c++"]),
      ],
      linkerSettings: [
        .linkedFramework("Foundation")
      ]
    ),
  ]
)
