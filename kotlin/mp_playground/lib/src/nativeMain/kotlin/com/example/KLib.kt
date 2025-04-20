package com.example

import kotlinx.cinterop.*

object Object {
  val field = "A"
}

interface Interface {
  fun iMember() {
    println(message = "Interface.iMember")
  }
}

data class DataClass(val field: String)

class Clazz : Interface {
  fun member(p: Int): ULong? = 42UL
}

fun printDataClass(data: DataClass) {
  println(message = "DataClass: ${data.field}")
}

fun forIntegers(b: Byte, s: UShort, i: Int, l: ULong?) { 
  println(message = "forIntegers: $b, $s, $i, $l")
}

fun forFloats(f: Float, d: Double?) {
  println(message = "forFloats: $f, $d")
 }

fun strings(str: String?) : String {
  return "That is '$str' from C"
}

fun acceptFun(f: (String) -> String?) = f("Kotlin/Native rocks!")
fun supplyFun() : (String) -> String? = { "$it is cool!" }

@kotlinx.cinterop.ExperimentalForeignApi
typealias CFun = CPointer<CFunction<(kotlin.Int) -> kotlin.Int>>

@kotlinx.cinterop.ExperimentalForeignApi
fun c_acceptFun(f: CFun): Int = f(42)

@kotlinx.cinterop.ExperimentalForeignApi
fun c_supplyFun() : CFun = staticCFunction<Int, Int> { it + 1 }