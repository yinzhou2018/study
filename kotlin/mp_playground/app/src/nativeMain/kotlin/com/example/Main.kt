package com.example

import kotlinx.cinterop.*
import platform.posix.*
import simple_lib.*

@OptIn(ExperimentalForeignApi::class)
fun main() {
  println("Testing C library functions:")

  // 测试add函数
  val result = add(5, 3)
  println("5 + 3 = $result")

  // 测试get_greeting函数
  val greeting = get_greeting()?.toKString() ?: "Failed to get greeting"
  println("Greeting from C: $greeting")

  // 调用一个posix线程函数
  val threadId = pthread_self()
  println("Current thread ID: $threadId")

  // 基于posix pthread接口创建一个线程做点事情，并等待它完成
  memScoped {
    val thread = alloc<pthread_tVar>()

    val threadFunc = staticCFunction { _: COpaquePointer? ->
      println("Hello from the thread! ID: ${pthread_self()}")
      null as COpaquePointer?
    }

    pthread_create(thread.ptr, null, threadFunc, null)
    pthread_join(thread.value, null)
  }
}
