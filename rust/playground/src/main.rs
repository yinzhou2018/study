// fn hello() {
//   crash(true);
// }

// fn crash(switch: bool) {
//   let i = if switch { 0 } else { 1 };
//   let j = 10 / i;
//   println!("{}", j);
// }

fn divide(a: i32, b: i32) -> Result<i32, &'static str> {
  if b == 0 {
    return Err("division by zero");
  }

  Ok(a / b)
}

fn call<F>(f: F) -> Result<i32, &'static str>
where
  F: Fn(i32, i32) -> Result<i32, &'static str>,
{
  let result = f(10, 2)?;
  println!("10 divided by 2 is {}", result);

  let result = f(10, 0)?;
  println!("10 divided by 0 is {}", result);
  return Ok(result);
}

fn get_lengther_one<'a>(left: &'a str, right: &'a str) -> &'a str {
  if left.len() > right.len() {
    return left;
  } else {
    return right;
  }
}

fn main() {
  // println!("Hello, world!");
  // hello();
  let result = call(divide).unwrap();
  println!("10 divided by 2 is {}", result);

  let result = call(divide).unwrap();
  println!("10 divided by 0 is {}", result);

  let s1 = "abc".to_string();
  // let left: &str;
  // let right: &str;
  let result: &str;
  {
    // let s1 = "abc".to_string();
    // let s2 = "cdfg".to_string();
    let s2 = "cdfg".to_string();
    let left = &s1;
    let right = &s2;
    result = get_lengther_one(left, right);
  }

  println!("{}", result);
}
