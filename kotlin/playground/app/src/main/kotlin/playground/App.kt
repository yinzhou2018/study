package playground

import kotlinx.coroutines.*

fun main() {
  println("start")
  // Executors.newSingleThreadExecutor().asCoroutineDispatcher().use { context ->
  runBlocking {
    withContext(Dispatchers.Default) { task1() }
    launch { task2() }
    println("called taskl and task2 from ${Thread.currentThread()}")
  }
  // }
  println("done")
}

suspend fun task1() {
  println("start task1 in Thread ${Thread.currentThread ()}")
  val time = calcTime()
  println("time: $time")
  delay(1000)
  // yield()
  println("end task1 in Thread ${Thread.currentThread ()}")
}

suspend fun task2() {
  println("start task2 in Thread ${Thread.currentThread ()}")
  yield()
  println("end task2 in Thread ${Thread.currentThread ()}")
}

suspend fun calcTime(): Int {
  println("start task3 in Thread ${Thread.currentThread ()}")
  return 1000
}
