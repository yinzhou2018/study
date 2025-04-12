// swift-tools-version: 5.10
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
  name: "playground",
  products: [
    .executable(name: "cfamily_bin", targets: ["cfamily_bin"]),
    .executable(name: "swift_bin", targets: ["swift_bin"]),
    .library(name: "swift_lib", targets: ["swift_lib"]),
    .library(name: "cfamily_lib", type: .dynamic, targets: ["cfamily_lib"]),
  ],
  targets: [
    .executableTarget(
      name: "swift_bin",
      dependencies: ["swift_lib", "cfamily_lib"],
    ),
    .executableTarget(
      name: "cfamily_bin",
      dependencies: ["swift_lib", "cfamily_lib"],
    ),
    .target(
      name: "swift_lib"
    ),
    .target(
      name: "cfamily_lib",
      publicHeadersPath: "./",
      linkerSettings: [
        .linkedFramework("Foundation")
      ]
    ),
  ]
)
