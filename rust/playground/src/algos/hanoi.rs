use std::fmt::{Display, Formatter, Result};

#[derive(Clone, Copy)]
enum HanoiPosition {
  PillarA,
  PillarB,
  PillarC,
}

impl Display for HanoiPosition {
  fn fmt(&self, f: &mut Formatter<'_>) -> Result {
    match *self {
      HanoiPosition::PillarA => {
        write!(f, "{}", "Pillar A")
      }
      HanoiPosition::PillarB => {
        write!(f, "{}", "Pillar B")
      }
      HanoiPosition::PillarC => {
        write!(f, "{}", "Pillar C")
      }
    }
  }
}

fn print_action(source: HanoiPosition, destination: HanoiPosition, step: i32) {
  println!("{}. {} => {}", step, source, destination);
}

fn hanoi_impl(n: i32, start: HanoiPosition, target: HanoiPosition, middle: HanoiPosition, step: i32) -> i32 {
  if n == 1 {
    print_action(start, target, step);
    return step + 1;
  }

  let mut next_step = hanoi_impl(n - 1, start, middle, target, step);
  print_action(start, target, next_step);
  next_step = next_step + 1;
  return hanoi_impl(n - 1, middle, target, start, next_step);
}

pub fn hanoi(n: i32) {
  hanoi_impl(n, HanoiPosition::PillarA, HanoiPosition::PillarC, HanoiPosition::PillarB, 1);
}
