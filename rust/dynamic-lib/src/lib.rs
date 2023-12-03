#[no_mangle]
pub extern "C" fn add(a: i32, b: i32) -> i32 {
  println!("Enter add...");
  crash(true);
  a + b
}

fn crash(switch: bool) {
  let i = if switch { 0 } else { 1 };
  let j = 10 / i;
  println!("{}", j);
}

#[cfg(test)]
mod tests {
  use super::*;

  #[test]
  fn it_works() {
    let result = add(2, 2);
    assert_eq!(result, 4);
  }
}
