use std::cmp::max;

// 完全0-1背包问题：给定一组物品的重量和价值，求最大价值，每个物品可以重复放入背包中，但是背包的重量限制为capacity
// 设f(n,c)表示到第n个物品，容量为c情况下的最大价值，w表示第n个物品重量，v表示第n个物品价值，状态转移方程为：
// f(n,c) = max{f(n-1,c), f(n-1,c-w)+v, f(n-1,c-2*w)+2*v, ...}
pub fn complete_bag_problem(weights_values: &[(i32, i32)], capacity: i32) -> i32 {
  let mut dp = vec![vec![0; capacity as usize + 1]; weights_values.len() + 1];
  for i in 1..weights_values.len() + 1 {
    for j in 1..capacity + 1 {
      let mut max_value = 0;
      let weight = weights_values[i - 1].0;
      let value = weights_values[i - 1].1;
      let mut count = 0;
      let mut cap = j;
      while cap >= 0 {
        let current_value = dp[i - 1][cap as usize] + value * count;
        if current_value > max_value {
          max_value = current_value;
        }
        count += 1;
        cap -= weight;
      }
      dp[i][j as usize] = max_value;
    }
  }
  return dp[weights_values.len()][capacity as usize];
}

// 完全0-1背包问题：给定一组物品的重量和价值，求最大价值，每个物品可以重复放入背包中，但是背包的重量限制为capacity
// 设f(n,c)表示到决策到第n个物品，容量为c情况下的最大价值，w表示第n个物品重量，v表示第n个物品价值，状态转移方程为：
// f(n,c) = max{f(n-1,c), f(n,c-w)+v} //在不包括第n个物品的最大值及至少包括一个第n个物品最大值间取大者
pub fn complete_bag_problem_v2(weights_values: &[(i32, i32)], capacity: i32) -> i32 {
  let mut dp = vec![vec![0; capacity as usize + 1]; weights_values.len() + 1];
  for i in 1..weights_values.len() + 1 {
    for j in 1..capacity + 1 {
      let left = dp[i - 1][j as usize];
      let mut right = 0;
      let cap = j - weights_values[i - 1].0;
      if cap >= 0 {
        right = dp[i][cap as usize] + weights_values[i - 1].1;
      }
      dp[i][j as usize] = max(left, right);
    }
  }
  return dp[weights_values.len()][capacity as usize];
}

// 字符串间最小编辑距离问题：输入两个字符串s跟t，求从s编辑到t所需的最小步骤，每个步骤编辑操作只能为
// 插入，替换，删除三个中间的一个，f(i,j)表示字符串s的前i个字串编辑为t的前j个字串所需的最小编辑次数，
// 则状态转换方程为：f(i,j) = min{f(i,j-1),f(i-1,j),f(i-1,j-1)} + 1，若s[i-1]=t[j-1],则：
// f(i,j) = f(i-1,j-1)
pub fn min_edit_distance(s: &str, t: &str) -> i32 {
  let mut dp = vec![vec![0; t.len() + 1]; s.len() + 1];
  (0..=s.len()).for_each(|i| dp[i][0] = i as i32);
  (0..=t.len()).for_each(|i| dp[0][i] = i as i32);
  for i in 1..=s.len() {
    for j in 1..=t.len() {
      if s.as_bytes()[i - 1] == t.as_bytes()[j - 1] {
        dp[i][j] = dp[i - 1][j - 1];
      } else {
        let mut min = dp[i][j - 1];
        if dp[i - 1][j] < min {
          min = dp[i - 1][j];
        }
        if dp[i - 1][j - 1] < min {
          min = dp[i - 1][j - 1];
        }
        dp[i][j] = min + 1;
      }
    }
  }
  return dp[s.len()][t.len()];
}
