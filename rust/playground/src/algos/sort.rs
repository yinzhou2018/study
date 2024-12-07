pub fn bubble_sort(ary: &mut [i32]) {
  for i in (1..ary.len()).rev() {
    for j in 0..i {
      if ary[j] > ary[j + 1] {
        (ary[j], ary[j + 1]) = (ary[j + 1], ary[j]);
      }
    }
  }
}

pub fn quick_sort(ary: &mut [i32]) {
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
