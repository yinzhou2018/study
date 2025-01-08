use std::cmp::max;

// 完全0-1背包问题：给定一组物品的重量和价值，求最大价值，每个物品可以重复放入背包中，但是背包的重量限制为capacity
// 设f(n,c)表示到第n个物品，容量为c情况下的最大价值，w表示第n个物品重量，v表示第n个物品价值，状态转移方程为：
// f(n,c) = max{f(n-1,c), f(n-1,c-w)+v, f(n-1,c-2*w)+2*v, ...}
pub fn knapsack_with_repetition(weights_values: &[(i32, i32)], capacity: i32) -> i32 {
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
pub fn knapsack_with_repetition_v2(weights_values: &[(i32, i32)], capacity: i32) -> i32 {
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
pub fn calculate_min_edit_distance(s: &str, t: &str) -> i32 {
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

// 最长公共子序列（不需要连续），f(i,j)表示以i位置结尾及以j位置结尾的两个字符串的最长公共子序列，状态转移方程为：
// f(i,j) = max{f(i-1,j), f(i,j-1), f(i-1,j-1)+ if s[i-1] == t[j-1] ? 1 : 0}
pub fn find_longest_common_subsequence(s: &str, t: &str) -> String {
  if s.is_empty() || t.is_empty() {
    return String::new();
  }

  let s_len = s.len() + 1;
  let t_len = t.len() + 1;
  let mut dp = vec![vec![Vec::new(); t_len]; s_len];
  for i in 1..=s.len() {
    for j in 1..=t.len() {
      let ref first = dp[i - 1][j];
      let first_len = first.len();
      let ref second = dp[i][j - 1];
      let second_len = second.len();
      let mut third = dp[i - 1][j - 1].clone();
      if s.as_bytes()[i - 1] == t.as_bytes()[j - 1] {
        third.push(i - 1);
      }
      let third_len = third.len();
      dp[i][j] = if first_len > second_len {
        if first_len > third_len {
          first.clone()
        } else {
          third
        }
      } else {
        if second_len > third_len {
          second.clone()
        } else {
          third
        }
      };
    }
  }

  let ref range = dp[s.len()][t.len()];
  let mut result = String::new();
  let s_bytes = s.as_bytes();
  for i in 0..range.len() {
    result.push(char::from_u32(s_bytes[range[i]] as u32).unwrap());
  }
  println!("{:?}", dp);
  return result;
}

// 最长公共子串（需要连续），f(i,j)表示以i位置结尾及以j位置结尾的两个字符串的最长公共子串，状态转移方程为：
// f(i,j) = max{f(i-1,j), f(i,j-1), 以i，j为结尾的子串}
pub fn find_longest_common_substring<'a>(s: &'a str, t: &'a str) -> &'a str {
  if s.is_empty() || t.is_empty() {
    return "";
  }

  // tuple(usize, usize, usize)类型说明:
  // [tuple.0, tuple.1)：截止当当前最长子串区间
  // tuple.2：以当前两个串尾为尾部的公共子串长度
  let mut dp = vec![vec![(0 as usize, 0 as usize, 0 as usize); t.len() + 1]; s.len() + 1];
  for i in 1..=s.len() {
    for j in 1..=t.len() {
      let ref first = dp[i - 1][j];
      let first_len = first.1 - first.0;
      let ref second = dp[i][j - 1];
      let second_len = second.1 - second.0;

      let ref third = dp[i - 1][j - 1];
      let mut third_len = third.2;
      if s.as_bytes()[i - 1] == t.as_bytes()[j - 1] {
        third_len += 1;
      } else {
        third_len = 0;
      }

      if first_len > second_len {
        if first_len > third_len {
          dp[i][j] = (first.0, first.1, third_len);
        } else {
          dp[i][j] = (i - third_len, i, third_len);
        }
      } else {
        if second_len > third_len {
          dp[i][j] = (second.0, second.1, third_len);
        } else {
          dp[i][j] = (i - third_len, i, third_len);
        }
      }
    }
  }

  println!("{:?}", dp);
  let ref range = dp[s.len()][t.len()];
  return &s[range.0..range.1];
}
