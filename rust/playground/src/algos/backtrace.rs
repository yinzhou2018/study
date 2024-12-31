use std::cmp::min;
use std::collections::HashSet;

// 获得数组全排序
pub fn all_sorts(ary: &[i32]) -> Vec<Vec<i32>> {
  let mut result = Vec::new();
  let mut path = Vec::new();
  all_sorts_impl(ary, &mut path, &mut result);
  return result;
}

fn all_sorts_impl(ary: &[i32], path: &mut Vec<i32>, result: &mut Vec<Vec<i32>>) {
  if ary.is_empty() {
    result.push(path.clone());
    return;
  }

  for i in 0..ary.len() {
    path.push(ary[i]);
    let mut new_ary = ary.to_vec();
    new_ary.remove(i);
    all_sorts_impl(&new_ary, path, result);
    path.pop();
  }
}

pub fn all_sorts_v2(ary: &[i32]) -> Vec<Vec<i32>> {
  let mut result = Vec::new();
  let mut selected: Vec<bool> = (0..ary.len()).map(|_| false).collect();
  let mut path = Vec::new();
  all_sorts_impl_v2(ary, &mut selected, &mut path, &mut result);
  return result;
}

fn all_sorts_impl_v2(ary: &[i32], selected: &mut Vec<bool>, path: &mut Vec<i32>, result: &mut Vec<Vec<i32>>) {
  if ary.len() == path.len() {
    result.push(path.clone());
    return;
  }

  let mut keys = HashSet::new();
  for i in 0..ary.len() {
    if keys.contains(&ary[i]) || selected[i] {
      continue;
    }
    keys.insert(ary[i]);
    path.push(ary[i]);
    selected[i] = true;
    all_sorts_impl_v2(ary, selected, path, result);
    path.pop();
    selected[i] = false;
  }
}

// 上楼梯方案
pub fn up_stairs_methods(n: i32) -> Vec<Vec<i32>> {
  let ary = [1, 2];
  let mut result = Vec::new();
  let mut path = Vec::new();
  up_stairs_methods_impl(&ary, n, &mut path, &mut result);
  return result;
}

fn up_stairs_methods_impl(ary: &[i32], n: i32, path: &mut Vec<i32>, result: &mut Vec<Vec<i32>>) {
  if n == 0 {
    result.push(path.clone());
    return;
  }

  if n < 0 {
    return;
  }

  for i in 0..ary.len() {
    path.push(ary[i]);
    up_stairs_methods_impl(ary, n - ary[i], path, result);
    path.pop();
  }
}

pub fn generate_ipv4(s: &str) -> Vec<String> {
  let mut ip_ary = Vec::new();
  if s.len() < 4 || s.len() > 12 {
    println!("Invalid IP address: {}, length should be between 4 and 12", s);
    return ip_ary;
  }

  let mut path = Vec::new();
  generate_ipv4_impl(s, &mut path, &mut ip_ary);

  return ip_ary;
}

fn is_valid_ipv4_part(s: &str) -> bool {
  if s.len() <= 3 {
    let part = s.parse::<i32>().unwrap();
    return part >= 0 && part <= 255;
  }

  return false;
}

fn generate_ipv4_impl(s: &str, path: &mut Vec<String>, result: &mut Vec<String>) {
  if path.len() == 3 {
    if is_valid_ipv4_part(s) {
      path.push(s.to_string());
      result.push(path.join("."));
      path.pop();
    }
    return;
  }

  for i in 0..min(s.len(), 3) {
    let part = &s[0..i + 1];
    if is_valid_ipv4_part(part) {
      path.push(part.to_string());
      if i + 1 < s.len() {
        generate_ipv4_impl(&s[i + 1..], path, result);
      }
      path.pop();
    }
  }
}
