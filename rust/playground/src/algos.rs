fn bubble_sort(ary: &mut [i32]) {
  for i in (1..ary.len()).rev() {
    for j in 0..i {
      if ary[j] > ary[j + 1] {
        (ary[j], ary[j + 1]) = (ary[j + 1], ary[j]);
      }
    }
  }
}

pub fn algos_study() {
  let mut ary = [1, 3, 2, 10, 5, 6];
  bubble_sort(&mut ary);
  println!("{:?}", ary);
}
