// The Swift Programming Language
// https://docs.swift.org/swift-book

protocol Printable {
  func print_1()
}

struct Printer: Printable {
  var name: String = "Printer"
  func print_1() {
    print("Hello, world: \(name)")
  }

  func print_2() {
    print("Hello, world, printer: \(name)")
  }
}

class Person: Printable {
  var name = "John"
  func print_1() {
    print("Hello, world: \(name)")
    name = "Jane"
  }

  var age: Int { 20 }
}

func print_2(_ p: any Printable) {
  // 获取类型内存大小
  print("Person size: \(MemoryLayout<Person>.size)")
  print("Printable size: \(MemoryLayout<Printable>.size(ofValue: p))")
  p.print_1()

  if var p = p as? Printer {
    p.print_2()
    withUnsafePointer(to: &p) {
      ptr in print("结构类型变量的内存地址: \(ptr)")
    }
  }
}

func rename(_ p: Person) {
  p.name = "Jane"
}

// var stepsize = 10
// func increment(_ number: inout Int) {
//   number += stepsize
// }
// increment(&stepsize)

var printer = Printer()
withUnsafePointer(to: &printer) {
  ptr in print("结构类型变量的内存地址: \(ptr)")
}
// var any_printer: any Printable = printer
print_2(printer)

var person = Person()
rename(person)
print_2(person)
print("Hello, world: \(person.name)")

var p = 10 {
  willSet {
    print("willSet")
  }
  didSet {
    print("didSet")
  }
}
print(p)
 p = 20
print(p)

print("10 + 10 = \(add(a: 10, b: 10))")

