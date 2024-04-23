fn bubble_sort(ary: &mut [i32]) {
  for i in (1..ary.len()).rev() {
    for j in 0..i {
      if ary[j] > ary[j + 1] {
        (ary[j], ary[j + 1]) = (ary[j + 1], ary[j]);
      }
    }
  }
}

fn quick_sort(ary: &mut [i32]) {
  let mut left = 0;
  let mut right = ary.len() - 1;
  let mut key_idx = (left + right) / 2;
  let key = ary[key_idx];

  // 循环不变式：`key_idx`指向本次选中key值所在位置，left左边值 <= key, right右边值 > key
  while left < right {
    if ary[left] > key {
      (ary[left], ary[right]) = (ary[right], ary[left]);
      right = right - 1;
      if ary[left] == key {
        key_idx = left;
      }
    } else {
      left = left + 1;
    }
  }

  // 循环完后left == right，需要再做一次与key值的比较交换
  if ary[left] < ary[key_idx] {
    (ary[left], ary[key_idx]) = (ary[key_idx], ary[left]);
  }

  if left > 0 {
    quick_sort(&mut ary[0..left]);
  }
  if left + 1 < ary.len() {
    let len = ary.len();
    quick_sort(&mut ary[left + 1..len]);
  }
}

fn generate_ipv4(s: &str) -> Vec<String> {
  let mut ip_ary = Vec::new();
  if s.len() < 4 || s.len() > 12 {
    println!("Invalid IP address: {}, length should be between 4 and 12", s);
    ip_ary
  } else {
    for i in 1..=std::cmp::min(3, s.len() - 3) {
      let ip1 = &s[0..i];
      if ip1.parse::<i32>().unwrap() > 255 {
        break;
      }
      for j in i + 1..=std::cmp::min(i + 3, s.len() - 2) {
        let ip2 = &s[i..j];
        if ip2.parse::<i32>().unwrap() > 255 {
          break;
        }
        for k in j + 1..=std::cmp::min(j + 3, s.len() - 1) {
          let ip3 = &s[j..k];
          let ip4 = &s[k..];
          if ip3.parse::<i32>().unwrap() > 255 {
            break;
          }
          if ip4.parse::<i32>().unwrap() > 255 {
            continue;
          }
          ip_ary.push(format!("{}.{}.{}.{}", ip1, ip2, ip3, ip4));
        }
      }
    }
    ip_ary
  }
}

pub fn algos_study() {
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    bubble_sort(&mut ary);
    println!("bubble sort: {:?}", ary);
  }
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    quick_sort(&mut ary);
    println!("quick sort: {:?}", ary);
  }

  let ip_ary = generate_ipv4("1221221221");
  println!("ip_ary: {:?}", ip_ary);
}
