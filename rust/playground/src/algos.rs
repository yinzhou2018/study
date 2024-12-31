mod backtrace;
mod hanoi;
mod math_expr_parser;
mod sort;
mod tree;

// mod linked_list;
use backtrace::*;
use hanoi::*;
use math_expr_parser::*;
use sort::*;
use tree::*;
// use linked_list::*;

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
  {
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

    let tree_root = make_tree_from_pre_mid_order(&[3, 9, 2, 1, 7], &[9, 3, 1, 2, 7]);
    dfs_post_tree_v2(tree_root.as_ref().unwrap());
    print!("\n");
  }

  hanoi(5);

  {
    let ary = [1, 2, 3, 4, 5];
    let result = all_sorts(&ary);
    let mut i = 1;
    result.iter().for_each(|path| {
      println!("{}: {:?}", i, path);
      i = i + 1;
    });
  }

  {
    let ary = [1, 1, 3, 4, 5];
    let result = all_sorts_v2(&ary);
    let mut i = 1;
    result.iter().for_each(|path| {
      println!("{}: {:?}", i, path);
      i = i + 1;
    });
  }

  {
    let result = up_stairs_methods(4);
    println!("result: {:?}", result);
  }

  {
    let ip_ary = generate_ipv4("1221221221");
    println!("ip_ary: {:?}", ip_ary);
  }
}
