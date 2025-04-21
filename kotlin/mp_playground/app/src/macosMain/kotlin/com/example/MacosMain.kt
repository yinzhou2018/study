package com.example

import kotlinx.cinterop.*
import macos_lib.*

@OptIn(ExperimentalForeignApi::class)
actual fun osMain() {
  println("Hello from macosMain!")

  val sum = MacosLib.add(5, 3)
  println("5 + 3 = $sum")

  val macosLibInstance = MacosLib()
  macosLibInstance.init(10)
  val diff = macosLibInstance.sub(4)
  println("10 - 4 = $diff")
}
