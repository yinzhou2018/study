package com.example

import simple_lib.*
import kotlinx.cinterop.*

@OptIn(ExperimentalForeignApi::class)
fun main() {
  println("Testing C library functions:")

  // 测试add函数
  val result = add(5, 3)
  println("5 + 3 = $result")

  // 测试get_greeting函数
  val greeting = get_greeting()?.toKString() ?: "Failed to get greeting"
  println("Greeting from C: $greeting")
}
