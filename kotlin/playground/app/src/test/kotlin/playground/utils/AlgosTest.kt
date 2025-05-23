package playground.utils

import org.junit.jupiter.api.Assertions.*
import org.junit.jupiter.api.Test
import org.junit.jupiter.api.assertDoesNotThrow

class AlgosTest {
  @Test
  fun `test quickSort with normal array`() {
    print("start test...")
    val arr = intArrayOf(5, 2, 8, 1, 9, 3)
    val expected = intArrayOf(1, 2, 3, 5, 8, 9)
    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(expected, arr)
  }

  @Test
  fun `test quickSort with empty array`() {
    val arr = intArrayOf()
    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(intArrayOf(), arr)
  }

  @Test
  fun `test quickSort with single element`() {
    val arr = intArrayOf(1)
    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(intArrayOf(1), arr)
  }

  @Test
  fun `test quickSort with duplicate elements`() {
    val arr = intArrayOf(3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5)
    val expected = intArrayOf(1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9)
    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(expected, arr)
  }

  @Test
  fun `test quickSort with already sorted array`() {
    val arr = intArrayOf(1, 2, 3, 4, 5)
    val expected = intArrayOf(1, 2, 3, 4, 5)
    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(expected, arr)
  }

  @Test
  fun `test quickSort with reverse sorted array`() {
    val arr = intArrayOf(5, 4, 3, 2, 1)
    val expected = intArrayOf(1, 2, 3, 4, 5)
    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(expected, arr)
  }

  @Test
  fun `test quickSort with negative numbers`() {
    val arr = intArrayOf(-5, 2, -8, 1, -9, 3)
    val expected = intArrayOf(-9, -8, -5, 1, 2, 3)
    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(expected, arr)
  }

  @Test
  fun `test quickSort with all same elements`() {
    val arr = intArrayOf(2, 2, 2, 2, 2)
    val expected = intArrayOf(2, 2, 2, 2, 2)
    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(expected, arr)
  }

  @Test
  fun `test quickSort with null array should not throw exception`() {
    assertDoesNotThrow {
      val arr = intArrayOf()
      quickSort(arr, 0, -1)
    }
  }

  @Test
  fun `test quickSort with invalid range should not affect array`() {
    val arr = intArrayOf(5, 4, 3, 2, 1)
    val original = arr.clone()

    // 测试开始索引大于结束索引的情况
    quickSort(arr, 3, 2)
    assertArrayEquals(original, arr)

    // 测试无效索引的情况，但要确保索引在有效范围内
    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(arr.sortedArray(), arr)
  }

  @Test
  fun `test quickSort with large array`() {
    val size = 10000
    val arr = IntArray(size) { (Math.random() * size).toInt() }
    val expected = arr.clone().sortedArray()

    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(expected, arr)
  }

  @Test
  fun `test quickSort with array containing Int MAX and MIN values`() {
    val arr = intArrayOf(Int.MAX_VALUE, Int.MIN_VALUE, 0, -1, 1)
    val expected = intArrayOf(Int.MIN_VALUE, -1, 0, 1, Int.MAX_VALUE)

    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(expected, arr)
  }

  @Test
  fun `test quickSort with alternating high and low values`() {
    val arr = intArrayOf(1, 10, 2, 9, 3, 8, 4, 7, 5, 6)
    val expected = intArrayOf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)

    quickSort(arr, 0, arr.size - 1)
    assertArrayEquals(expected, arr)
  }
}
