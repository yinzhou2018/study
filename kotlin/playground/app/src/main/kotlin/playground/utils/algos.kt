package playground.utils

// 找到第一个只出现一次的字符
// 输入："abaccdeff"
// 输出：'b'
// 输入：""
// 输出：' '
fun dismantlingAction(arr: String): Char {
  val map = linkedMapOf<Char, Int>() // 有序的Map，遍历顺序与插入顺序一致
  arr.forEach { map[it] = map.getOrDefault(it, 0) + 1 }
  return map.firstNotNullOfOrNull { if (it.value == 1) it.key else null } ?: ' '
}

// scores 是一个非严格递增数组，target 是要查找的目标值，返回 target 在 scores 中出现的次数
fun countTarget(scores: IntArray, target: Int): Int {
  var left = 0
  var right = scores.size - 1
  var count = 0
  while (left <= right) {
    val mid = (left + right) / 2
    if (scores[mid] == target) {
      count++
      var i = mid - 1
      while (i >= left && scores[i] == target) {
        count++
        i--
      }
      i = mid + 1
      while (i <= right && scores[i] == target) {
        count++
        i++
      }
      break
    } else if (scores[mid] < target) {
      left = mid + 1
    } else {
      right = mid - 1
    }
  }
  return count
}

// records 是一个非严格递增数组，返回第一个缺失的整数
fun takeAttendance(records: IntArray): Int {
  var left = 0
  var right = records.size - 1
  while (left <= right) {
    val mid = (left + right) / 2
    if (mid == records[mid]) {
      left = mid + 1
    } else {
      right = mid - 1
    }
  }

  return left
}

// 在向左及向下非严格递增的二维数组中查找目标值
fun findTargetIn2DPlants(plants: Array<IntArray>, target: Int): Boolean {
  if (plants.isEmpty()) {
    return false
  }

  var rowPos = 0
  var colPos = 0
  var rowSize = plants.size
  var colSize = plants[0].size

  data class Range(val rowPos: Int, val colPos: Int, val size: Int, val isRowSize: Boolean)

  while (rowSize > 0 && colSize > 0) {
    // 比最小值还小或者比最大值还大，肯定找不到目标值
    if (target < plants[rowPos][colPos] || target > plants[rowPos + rowSize - 1][colPos + colSize - 1]) {
      return false
    }

    // 四条边构成四个区间
    val ranges =
        arrayOf(
            Range(rowPos, colPos, colSize, false),
            Range(rowPos, colPos, rowSize, true),
            Range(rowPos, colPos + colSize - 1, rowSize, true),
            Range(rowPos + rowSize - 1, colPos, colSize, false)
        )

    // 在四个区间中二分查找目标值
    for (range in ranges) {
      var left = 0
      var right = range.size - 1
      while (left <= right) {
        val mid = (left + right) / 2
        val value =
            if (range.isRowSize) plants[range.rowPos + mid][range.colPos] else plants[range.rowPos][range.colPos + mid]
        if (value == target) {
          return true
        } else if (value < target) {
          left = mid + 1
        } else {
          right = mid - 1
        }
      }
    }

    rowPos++
    colPos++
    rowSize -= 2
    colSize -= 2
  }

  return false
}

fun testFindTargetIn2DPlants() {
  val plants = arrayOf(intArrayOf(2, 3, 6, 8), intArrayOf(4, 5, 8, 9), intArrayOf(5, 9, 10, 12))
  val target = 8
  val found = findTargetIn2DPlants(plants, target)
  println("Target $target found: $found")
}

// 在二维网格中查找单词: 回溯搜索
fun wordPuzzle(grid: Array<CharArray>, target: String): Boolean {
  if (grid.isEmpty() || grid[0].isEmpty() || target.isEmpty()) {
    return false
  }

  val rows = grid.size
  val cols = grid[0].size
  for (i in 0 ..< rows) {
    for (j in 0 ..< cols) {
      val path = mutableListOf(Pair(i, j))
      if (recWordPuzzle(grid, path, calcChoices(rows, cols, path), target, 0)) {
        return true
      }
    }
  }

  return false
}

fun calcChoices(rows: Int, cols: Int, path: List<Pair<Int, Int>>): List<Pair<Int, Int>> {
  val (x, y) = path.last()
  return arrayOf(Pair(x - 1, y), Pair(x + 1, y), Pair(x, y - 1), Pair(x, y + 1)).filter {
    it.first >= 0 && it.first < rows && it.second >= 0 && it.second < cols && !path.contains(it)
  }
}

fun recWordPuzzle(
    grid: Array<CharArray>,
    path: MutableList<Pair<Int, Int>>,
    choices: List<Pair<Int, Int>>,
    target: String,
    index: Int
): Boolean {
  if (grid[path.last().first][path.last().second] != target[index]) {
    return false
  }

  if (index == target.length - 1) {
    return true
  }

  for (choice in choices) {
    path.add(choice)
    if (recWordPuzzle(grid, path, calcChoices(grid.size, grid[0].size, path), target, index + 1)) {
      return true
    }
    path.removeLast()
  }

  return false
}

fun testWordPuzzle() {
  val grid = arrayOf(charArrayOf('A', 'B', 'C', 'E'), charArrayOf('S', 'F', 'C', 'S'), charArrayOf('A', 'D', 'E', 'E'))
  val target = "SEE"
  val found = wordPuzzle(grid, target)
  println("Target $target found: $found")
}

// https://leetcode.cn/leetbook/read/illustration-of-algorithm/9h6vo2/
fun wardrobeFinishing(m: Int, n: Int, cnt: Int): Int {
  var path = mutableListOf<Pair<Int, Int>>()
  if (cnt >= 0) {
    recWardrobeFinishing(m, n, 0, 0, path, cnt)
  }

  return path.size
}

fun recWardrobeFinishing(rows: Int, cols: Int, i: Int, j: Int, path: MutableList<Pair<Int, Int>>, cnt: Int) {
  path.add(Pair(i, j))
  val choices = calcChoices(rows, cols, i, j, cnt, path)
  for (choice in choices) {
    recWardrobeFinishing(rows, cols, choice.first, choice.second, path, cnt)
  }
}

fun calcChoices(rows: Int, cols: Int, i: Int, j: Int, cnt: Int, path: List<Pair<Int, Int>>): List<Pair<Int, Int>> {
  return listOf(Pair(i + 1, j), Pair(i, j + 1)).filter {
    it.first < rows && it.second < cols && !path.contains(it) && calcSum(it.first) + calcSum(it.second) <= cnt
  }
}

fun calcSum(x: Int): Int {
  var sum = 0
  var m = x
  while (m > 0) {
    sum += m % 10
    m /= 10
  }
  return sum
}

fun quickSort(ary: IntArray, left: Int, right: Int) {
  if (left >= right) {
    return
  }

  var i = left
  var j = right - 1
  val key = ary[right]

  // 循环不变式为：所有索引大于j的元素 >= key, 所有索引小于i的元素 < key
  // 这里的循环条件是<=而不是<保证了退出循环时i > j, 从而i位置的元素一定>=key，
  // 后面不需要再做判断，而可以直接与key位置进行交换
  while (i <= j) {
    while (i <= j && ary[i] < key) {
      i++
    }
    while (i <= j && ary[j] >= key) {
      j--
    }

    if (i < j) {
      val temp = ary[i]
      ary[i] = ary[j]
      ary[j] = temp
    }
  }

  ary[right] = ary[i]
  ary[i] = key
  quickSort(ary, left, i - 1)
  quickSort(ary, i + 1, right)
}

