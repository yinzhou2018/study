package playground

import io.ktor.client.*
import io.ktor.client.call.*
import io.ktor.client.engine.cio.*
import io.ktor.client.plugins.contentnegotiation.*
import io.ktor.client.request.*
import io.ktor.serialization.kotlinx.json.*
import kotlinx.coroutines.*
import kotlinx.coroutines.runBlocking

fun main() = runBlocking {
  val client = HttpClient(CIO) { install(ContentNegotiation) { json() } }
  // GET 请求示例
  val getResponse: String = client.get("https://example.com").body()
  println("GET 请求响应: $getResponse")
  // POST 请求示例
  val postResponse: String = client.post("https://example.com") { setBody("{\"key\": \"value\"}") }.body()
  println("POST 请求响应: $postResponse")
  client.close()
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
