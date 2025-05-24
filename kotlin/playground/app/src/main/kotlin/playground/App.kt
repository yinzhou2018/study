package playground

import io.ktor.client.*
import io.ktor.client.call.*
import io.ktor.client.engine.cio.*
import io.ktor.client.plugins.contentnegotiation.*
import io.ktor.client.request.*
import io.ktor.serialization.kotlinx.json.*
import kotlinx.coroutines.*
<<<<<<< HEAD
import playground.utils.testGoodsOrder

fun main() {
  testGoodsOrder()
  return
  
=======

open class Animal {
  open var name: String = ""
  var age: Int = 0
  open fun sayHello() {
    println("Hello, I am $name, $age years old") 
  }
}

class Cat: Animal() {
  override var name: String = ""
  override fun sayHello() {
    println("Meow, I am $name, $age years old")
  } 
}

fun main() {
>>>>>>> 24372f8e532096b0055e906ded3d9b27b7b92e2f
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
