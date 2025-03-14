package playground

// import utils
import kotlinx.coroutines.*

fun main(args: Array<String>) {
  val a = utils.add(1, 2)
  println("Hello World: ${args.joinToString()}")
  println("${a}")
  runBlocking {
    val result = async {
      delay(1000L)
      println("World!")
      10
    }
    val b = result.await()
    println("result:${b}")
  }
}


