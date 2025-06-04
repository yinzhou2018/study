struct Algos {

  static func add(_ a: Int, _ b: Int) -> Int {
    return a + b
  }

  static func fib(_ n: Int) -> Int {
    if n == 0 || n == 1 {
      return n
    }
    var a = 0
    var b = 1
    for _ in 2...n {
      let tmp = a
      a = b
      b = tmp + a
    }
    return b
  }

  // https://leetcode.cn/leetbook/read/illustration-of-algorithm/59gq9c/
  static func maxSales(_ sales: [Int]) -> Int {
    if sales.count == 0 {
      return 0
    }
    var pf = sales[0]
    var maxValue = sales[0]
    for i in 1..<sales.count {
      let cf = max(pf + sales[i], sales[i])
      if cf > maxValue {
        maxValue = cf
      }
      pf = cf
    }
    return maxValue
  }

  // https://leetcode.cn/leetbook/read/illustration-of-algorithm/5vokvr/
  static func jewelleryValue(_ frame: [[Int]]) -> Int {
    var dp = [[Int]](repeating: [Int](repeating: 0, count: frame[0].count), count: frame.count)
    dp[0][0] = frame[0][0]

    for i in 1..<frame.count {
      dp[i][0] = dp[i - 1][0] + frame[i][0]
    }
    for j in 1..<frame[0].count {
      dp[0][j] = dp[0][j - 1] + frame[0][j]
    }
    for i in 1..<frame.count {
      for j in 1..<frame[0].count {
        dp[i][j] = max(dp[i - 1][j], dp[i][j - 1]) + frame[i][j]
      }
    }

    return dp[frame.count - 1][frame[0].count - 1]
  }
}
