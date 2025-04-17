import Foundation

@_cdecl("swift_add")
public func swiftAdd(_ a: Int32, _ b: Int32) -> Int32 {
  return a + b
}

@objc public class SwiftClass: NSObject {
  var value: Int32
  
  @objc public init(value: Int32) {
    self.value = value
  }

  @objc public func add(_ other: SwiftClass) -> Int32 {
    return self.value + other.value
  }
}
