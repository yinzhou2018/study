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

  // 循环不变式：`key_idx`指向本次选中key值所在位置，left左边的值 < key, right右边的值 > key
  while left < right {
    if ary[left] > key {
      if ary[right] == key {
        key_idx = left;
      }
      (ary[left], ary[right]) = (ary[right], ary[left]);
      right = right - 1;
    } else {
      left = left + 1;
    }
  }

  // 循环完后left == right，left左边小于或等于key，left右边大于key，需要在做一次与key值的比较交换
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
}
