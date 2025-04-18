package com.example

import kotlinx.cinterop.*

object Object {
  val field = "A"
}

interface Interface {
  fun iMember() {}
}

class Clazz : Interface {
  fun member(p: Int): ULong? = 42UL
}

fun forIntegers(b: Byte, s: UShort, i: Int, l: ULong?) { }
fun forFloats(f: Float, d: Double?) { }

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