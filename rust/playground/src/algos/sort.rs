pub fn bubble_sort(ary: &mut [i32]) {
  // 每次迭代冒出一个最大的放到ary[i]上，迭代ary.len - 1次则完成排序
  for i in (1..ary.len()).rev() {
    // 每次迭代后ary[j+1]是[0,j+1]区间最大元素，循环完冒泡出最大元素放到ary[i]
    for j in 0..i {
      if ary[j] > ary[j + 1] {
        (ary[j], ary[j + 1]) = (ary[j + 1], ary[j]);
      }
    }
  }
}

pub fn bubble_sort_v2(ary: &mut [i32]) {
  if ary.len() <= 1 {
    return;
  }

  for i in 0..ary.len() - 1 {
    if ary[i] > ary[i + 1] {
      (ary[i], ary[i + 1]) = (ary[i + 1], ary[i]);
    }
  }

  let len = ary.len();
  bubble_sort_v2(&mut ary[0..len - 1]);
}

pub fn select_sort(ary: &mut [i32]) {
  for i in (1..ary.len()).rev() {
    let mut max_idx = 0;
    for j in 1..=i {
      if ary[j] > ary[max_idx] {
        max_idx = j;
      }
    }
    (ary[max_idx], ary[i]) = (ary[i], ary[max_idx]);
  }
}

pub fn select_sort_v2(ary: &mut [i32]) {
  if ary.len() <= 1 {
    return;
  }

  let mut max_value = ary[0];
  let mut max_idx = 0;
  for i in 1..ary.len() {
    if ary[i] > max_value {
      max_value = ary[i];
      max_idx = i;
    }
  }
  let len = ary.len();
  (ary[max_idx], ary[len - 1]) = (ary[len - 1], ary[max_idx]);
  select_sort_v2(&mut ary[0..len - 1]);
}

pub fn insert_sort(ary: &mut [i32]) {
  for i in 1..ary.len() {
    let key = ary[i];
    let mut idx = i;
    for j in (0..=i - 1).rev() {
      if ary[j] > key {
        idx = j;
        ary[j + 1] = ary[j];
      } else {
        break;
      }
    }
    ary[idx] = key;
  }
}

// 自上而下堆化（取出堆顶元素：相当于交换首尾元素取出尾部元素，然后重新自上而下堆化）
fn uptodown_heapfy(ary: &mut [i32], start_idx: usize, end_idx: usize) {
  let mut parent_idx = start_idx;
  let mut left_child_idx = 2 * parent_idx + 1;
  let mut rigth_child_idx = 2 * parent_idx + 2;
  while left_child_idx <= end_idx {
    let mut max_idx = parent_idx;
    if ary[left_child_idx] > ary[max_idx] {
      max_idx = left_child_idx;
    }
    if rigth_child_idx <= end_idx && ary[rigth_child_idx] > ary[max_idx] {
      max_idx = rigth_child_idx;
    }
    if max_idx != parent_idx {
      (ary[max_idx], ary[parent_idx]) = (ary[parent_idx], ary[max_idx]);
      parent_idx = max_idx;
      left_child_idx = 2 * parent_idx + 1;
      rigth_child_idx = 2 * parent_idx + 2;
    } else {
      break;
    }
  }
}

// 自下而上堆化（插入元素：尾部插入，然后自下而上堆化）
fn downtoup_heapfy(ary: &mut [i32]) {
  let mut current_idx = (ary.len() - 1) as i32;
  let mut parent_idx = if current_idx % 2 == 0 { current_idx / 2 - 1 } else { current_idx / 2 };
  while parent_idx >= 0 {
    if ary[current_idx as usize] > ary[parent_idx as usize] {
      (ary[current_idx as usize], ary[parent_idx as usize]) = (ary[parent_idx as usize], ary[current_idx as usize]);
      current_idx = parent_idx;
      parent_idx = if current_idx % 2 == 0 { current_idx / 2 - 1 } else { current_idx / 2 };
    } else {
      break;
    }
  }
}

// 堆是一个完全二叉树，底层适合用数组存储
// 完美二叉树：所有层节点都是满的
// 完全二叉树：除了最后一层其他层节点都是满，且最后一层节点靠左排列
// 完满二叉树：所有非叶子节点都有两个子节点
pub fn heap_sort(ary: &mut [i32]) {
  //1. 先就地建最大堆，反向从最后一个非叶子节点向下堆化
  let end_idx = ary.len() - 1;
  let parent_idx = if end_idx % 2 == 0 { end_idx / 2 - 1 } else { end_idx / 2 };
  for i in (0..=parent_idx).rev() {
    uptodown_heapfy(ary, i, end_idx);
  }

  //2. 堆顶元素与尾部元素互换，重新自上而下堆化，重复ary.len - 1次则完成排序
  for i in (1..ary.len()).rev() {
    (ary[0], ary[i]) = (ary[i], ary[0]);
    uptodown_heapfy(ary, 0, i - 1);
  }
}

pub fn heap_sort_v2(ary: &mut [i32]) {
  //1. 开辟新空间插入建堆
  let mut new_ary: Vec<i32> = vec![];
  ary.iter().for_each(|e| {
    new_ary.push(*e);
    downtoup_heapfy(&mut new_ary);
  });

  //2. 堆顶元素与尾部元素互换，重新自上而下堆化，重复ary.len - 1次则完成排序
  for i in (1..new_ary.len()).rev() {
    (new_ary[0], new_ary[i]) = (new_ary[i], new_ary[0]);
    uptodown_heapfy(&mut new_ary, 0, i - 1);
  }

  let mut i = 0;
  new_ary.iter().for_each(|e| {
    ary[i] = *e;
    i = i + 1;
  });
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

  // 循环完后left == right，这个值可能小于key，所以需要再做一次与key的比较交换
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

pub fn quick_sort_v2(ary: &mut [i32]) {
  if ary.len() < 2 {
    return;
  }

  let mut left = 0;
  let mut right = ary.len() - 2;
  let key = ary[right + 1];

  // 循环不变式：left左边的都小于key, right右边的都大于等于key
  while left < right {
    while left < right && ary[left] < key {
      left = left + 1;
    }

    while left < right && ary[right] >= key {
      right = right - 1;
    }

    if left < right {
      (ary[left], ary[right]) = (ary[right], ary[left]);
    }
  }

  // 循环结束后left == right, left所在元素可能小于key值
  if ary[left] < key {
    left = left + 1;
  }
  let len = ary.len();
  (ary[left], ary[len - 1]) = (ary[len - 1], ary[left]);

  quick_sort_v2(&mut ary[0..left]);
  quick_sort_v2(&mut ary[left + 1..len]);
}
