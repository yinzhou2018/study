fn main() {
  println!("Hello, world!");
  hello();
}

fn hello() {
  crash(true);
}

fn crash(switch: bool) {
  let i = if switch { 0 } else { 1 };
  let j = 10 / i;
  println!("{}", j);
}
