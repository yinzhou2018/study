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

fn main() {
  // println!("Hello, world!");
  // hello();
  let result = call(divide).unwrap();
  println!("10 divided by 2 is {}", result);

  let result = call(divide).unwrap();
  println!("10 divided by 0 is {}", result);
}
