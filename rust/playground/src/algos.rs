mod sort;
mod ipv4_generator;
mod math_expr_parser;
use sort::*;
use ipv4_generator::*;
use math_expr_parser::*;


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

  let expr = "1+22*3";
  let result = parse_math_expr(expr);
  println!("expr: {}, result: {}", expr, result);
}
