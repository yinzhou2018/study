import cfamily_lib
import swift_lib

print("Hello, world!")
let r = swiftAdd(10, 20)
print("Result of swiftAdd: \(r)")
let obj1 = SwiftClass(value: 10)
let a = obj1.add(SwiftClass(value: 20))
print("Result of SwiftClass.add: \(a)")

let r1 = lib_add(10, 20)
print("Result of lib_add: \(r1)")
let obj2 = OCLib()
let r2 = obj2.add(10, with: 20)
print("Result of OCLib.add: \(r2)")

var d1 = LibData(a: 1, b: 2)
var d2 = LibData(a: 3, b: 4)
let d3 = lib_add_v1(d1, d2)
print("Result of lib_add_v1: \(d3.a), \(d3.b)")
let d4 = withUnsafeMutablePointer(to: &d1) { p1 in
  withUnsafeMutablePointer(to: &d2) { p2 in
    lib_add_v3(p1, p2)
  }
}
print("Result of lib_add_v3: \(d4.a), \(d4.b)")

print("Result is: \(Algos.fib(4))")
let hr = Algos.maxSales([-2, 1, -3, 4, -1, 2, 1, -5, 4])
print("Result of maxSales: \(hr)")
