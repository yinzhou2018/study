mod ipv4_generator;
mod math_expr_parser;
mod sort;
mod tree;
// mod linked_list;
use ipv4_generator::*;
use math_expr_parser::*;
use sort::*;
use tree::*;
// use linked_list::*;

struct Data(i32);

pub fn algos_study() {
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    bubble_sort(&mut ary);
    println!("bubble sort: {:?}", ary);
  }
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    bubble_sort_v2(&mut ary);
    println!("bubble sort v2: {:?}", ary);
  }
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    select_sort(&mut ary);
    println!("select sort: {:?}", ary);
  }
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    select_sort_v2(&mut ary);
    println!("select sort v2: {:?}", ary);
  }
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    insert_sort(&mut ary);
    println!("insert sort: {:?}", ary);
  }
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    quick_sort(&mut ary);
    println!("quick sort: {:?}", ary);
  }
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    quick_sort_v2(&mut ary);
    println!("quick sort v2: {:?}", ary);
  }
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    heap_sort(&mut ary);
    println!("heap sort: {:?}", ary);
  }
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    heap_sort_v2(&mut ary);
    println!("heap sort v2: {:?}", ary);
  }
  {
    let mut ary = [1, 3, 2, 10, 5, 6, 3, 4, 7, 11, 7, 8];
    merge_sort(&mut ary);
    println!("merge sort: {:?}", ary);
  }
  let ip_ary = generate_ipv4("1221221221");
  println!("ip_ary: {:?}", ip_ary);

  let expr = "1+22*3";
  let result = parse_math_expr_without_parentheses(expr);
  println!("{} = {}", expr, result);
  let expr = "(1+2)*(3*(6+4))";
  let result = parse_math_expr(expr);
  println!("{} = {}", expr, result);

  // let mut list = linked_list::LinkedList::new();
  // list.push_front(1);
  // list.push_front(2);
  // list.push_front(3);

  let tree_root = make_tree(&[1, 2, 3, 4, 5, 6, 7]);
  bfs_tree(tree_root.as_ref().unwrap());
  dfs_pre_tree(tree_root.as_ref().unwrap());
  print!("\n");
  dfs_pre_tree_v2(tree_root.as_ref().unwrap());
  print!("\n");
  dfs_mid_tree(tree_root.as_ref().unwrap());
  print!("\n");
  dfs_mid_tree_v2(tree_root.as_ref().unwrap());
  print!("\n");
  dfs_post_tree(tree_root.as_ref().unwrap());
  print!("\n");
  dfs_post_tree_v2(tree_root.as_ref().unwrap());
  print!("\n");
}
