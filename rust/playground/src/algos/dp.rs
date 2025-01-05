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
// f(n,c) = max{f(n-1,c), f(n,c-w)+v}
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
