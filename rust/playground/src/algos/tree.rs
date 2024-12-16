use std::collections::VecDeque;

use async_std::stream::Scan;

pub struct TreeNode {
  value: i32,
  left_child: Option<Box<TreeNode>>,
  right_child: Option<Box<TreeNode>>,
}

impl Drop for TreeNode {
  fn drop(&mut self) {
    println!("Tree Node free...");
  }
}

// 基于数组存储的完全二叉树构造链式存储二叉树,采用层次遍历（广度优先遍历）类似的思想，借助一个队列：
// 1. 队列里存放的是当前已经接入树但孩子还没全部构造好的节点
// 2. 每次从队头取节点做为父节点来挂载左右子节点, 然后将新挂载的节点入队列
// 3. 如果当前父节点两个孩子都有了则弹出队列
pub fn make_tree(values: &[i32]) -> Option<Box<TreeNode>> {
  let mut tree_root: Option<Box<TreeNode>> = None;
  let mut queue: VecDeque<*mut TreeNode> = VecDeque::new();
  values.iter().for_each(|e| {
    let mut node = Box::new(TreeNode { value: *e, left_child: None, right_child: None });
    let node_ref: &mut TreeNode = &mut node;
    let node_pointer = node_ref as *mut TreeNode;
    if queue.is_empty() {
      tree_root = Some(node);
    } else {
      let top_pointer = *queue.front().unwrap();
      let top_ref = unsafe { &mut *top_pointer };
      if top_ref.left_child.is_none() {
        top_ref.left_child = Some(node);
      } else {
        top_ref.right_child = Some(node);
        queue.pop_front();
      }
    }
    queue.push_back(node_pointer);
  });
  return tree_root;
}

// 输入前序及中序遍历构建二叉树，基于分治策略：
// 1. 取出前序数组首元素为树根，在中序数组里找到树根位置
// 2. 基于树根在中序数组里的位置则可以一分为二拆分出左子树前序+左子树中序以及右子树前序+右子树中序两个相同的子问题
// 3. 递归调用左右子树
pub fn make_tree_from_pre_mid_order(pre_order: &[i32], mid_order: &[i32]) -> Option<Box<TreeNode>> {
  if pre_order.len() != mid_order.len() || pre_order.is_empty() {
    return None;
  }

  let mut node = Box::new(TreeNode { value: pre_order[0], left_child: None, right_child: None });
  if pre_order.len() == 1 {
    return Some(node);
  }

  let mut index = 0;
  for i in 0..mid_order.len() {
    if mid_order[i] == pre_order[0] {
      index = i;
      break;
    }
  }
  let pre_left_child = &pre_order[1..index + 1];
  let mid_left_child = &mid_order[0..index];
  let pre_right_child = &pre_order[index + 1..];
  let mid_right_child = &mid_order[index + 1..];
  node.left_child = make_tree_from_pre_mid_order(pre_left_child, mid_left_child);
  node.right_child = make_tree_from_pre_mid_order(pre_right_child, mid_right_child);
  return Some(node);
}

// 层次遍历：借助队列来拉直遍历节点，不断从对头取遍历节点，并当前当前遍历节点的左右子节点入队列，循环往复
pub fn bfs_tree(tree_root: &Box<TreeNode>) {
  let mut queue: VecDeque<*const TreeNode> = VecDeque::new();
  let r_root = &tree_root as &TreeNode;
  queue.push_back(r_root as *const TreeNode);
  while !queue.is_empty() {
    let node_pointer = queue.pop_front().unwrap();
    let node_ref = unsafe { &*node_pointer };
    print!("{}, ", node_ref.value);
    if !node_ref.left_child.is_none() {
      let r_box = node_ref.left_child.as_ref().unwrap();
      let r_node = &r_box as &TreeNode;
      queue.push_back(r_node as *const TreeNode);
    }
    if !node_ref.right_child.is_none() {
      let r_box = node_ref.right_child.as_ref().unwrap();
      let r_node = &r_box as &TreeNode;
      queue.push_back(r_node as *const TreeNode);
    }
  }
  print!("\n");
}

// 递归先序遍历
pub fn dfs_pre_tree(tree_root: &Box<TreeNode>) {
  print!("{}, ", tree_root.value);
  if !tree_root.left_child.is_none() {
    dfs_pre_tree(tree_root.left_child.as_ref().unwrap());
  }
  if !tree_root.right_child.is_none() {
    dfs_pre_tree(tree_root.right_child.as_ref().unwrap());
  }
}

// 非递归先序遍历：借助显性栈结构来模拟递归，递归隐形借助了函数调用栈结构
pub fn dfs_pre_tree_v2(tree_root: &Box<TreeNode>) {
  let mut stack: Vec<*const TreeNode> = Vec::new();
  let node_ref = tree_root as &TreeNode;
  stack.push(node_ref as *const TreeNode);
  while !stack.is_empty() {
    let node_pointer = stack.pop().unwrap();
    let node_ref = unsafe { &*node_pointer };
    print!("{}, ", node_ref.value);
    if !node_ref.right_child.is_none() {
      let node_ref = node_ref.right_child.as_ref().unwrap() as &TreeNode;
      stack.push(node_ref as *const TreeNode);
    }
    if !node_ref.left_child.is_none() {
      let node_ref = node_ref.left_child.as_ref().unwrap() as &TreeNode;
      stack.push(node_ref as *const TreeNode);
    }
  }
}

// 中序遍历
pub fn dfs_mid_tree(tree_root: &Box<TreeNode>) {
  if !tree_root.left_child.is_none() {
    dfs_mid_tree(tree_root.left_child.as_ref().unwrap());
  }
  print!("{}, ", tree_root.value);
  if !tree_root.right_child.is_none() {
    dfs_mid_tree(tree_root.right_child.as_ref().unwrap());
  }
}

// 非递归中序遍历：类似非递归先序遍历思路，核心是需要一个状态变量来表示当前是处于递归下降过程还是递归回溯过程
// 一开始处于递归下降过程时，从栈顶获取元素并不断获取左子节点入栈
// 一旦下降到叶子节点，则将状态调整为递归回溯过程
// 处于递归回溯过程时，弹出栈顶元素输出值，并取右子节点，若为空，则继续向上回溯，若有，则将其入栈并将状态切回递归下降过程
pub fn dfs_mid_tree_v2(tree_root: &Box<TreeNode>) {
  let mut stack: Vec<*const TreeNode> = Vec::new();
  let node_ref = tree_root as &TreeNode;
  stack.push(node_ref as *const TreeNode);
  let mut backtracing = false;
  while !stack.is_empty() {
    let node_pointer = stack[stack.len() - 1];
    let node_ref = unsafe { &*node_pointer };
    if backtracing {
      stack.pop();
      print!("{}, ", node_ref.value);
      if !node_ref.right_child.is_none() {
        let node_ref = node_ref.right_child.as_ref().unwrap() as &TreeNode;
        stack.push(node_ref as *const TreeNode);
        backtracing = false;
      }
    } else if !node_ref.left_child.is_none() {
      let node_ref = node_ref.left_child.as_ref().unwrap() as &TreeNode;
      stack.push(node_ref as *const TreeNode);
    } else {
      backtracing = true;
    }
  }
}

// 后序遍历
pub fn dfs_post_tree(tree_root: &Box<TreeNode>) {
  if !tree_root.left_child.is_none() {
    dfs_post_tree(tree_root.left_child.as_ref().unwrap());
  }
  if !tree_root.right_child.is_none() {
    dfs_post_tree(tree_root.right_child.as_ref().unwrap());
  }
  print!("{}, ", tree_root.value);
}

#[derive(Copy, Clone)]
enum ScanState {
  ScanLeft,
  ScanRight,
  ScanSelf,
}

// 非递归后序遍历：类似非递归先序遍历思路，核心是每个入栈的节点需要一个状态变量配对，来指示出栈后不同操作,
// 每个入栈节点经历如下3个状态：
// 最开始为ScanLeft，行动为向下扫描他的左子树，然后进入ScanRight状态
// 处于ScanRight，行动为向下扫描他的又子树，然后进入ScanSelf状态
// 处于ScanSelf，行动为输出自己，并出栈
pub fn dfs_post_tree_v2(tree_root: &Box<TreeNode>) {
  let mut stack: Vec<(*const TreeNode, ScanState)> = Vec::new();
  let node_ref = tree_root as &TreeNode;
  stack.push((node_ref as *const TreeNode, ScanState::ScanLeft));
  while !stack.is_empty() {
    let len = stack.len();
    let (node_pointer, state) = stack[len - 1];
    let node_ref = unsafe { &*node_pointer };
    match state {
      ScanState::ScanSelf => {
        print!("{}, ", node_ref.value);
        stack.pop();
      }
      ScanState::ScanLeft => {
        if !node_ref.left_child.is_none() {
          let node_ref = node_ref.left_child.as_ref().unwrap() as &TreeNode;
          stack.push((node_ref as *const TreeNode, ScanState::ScanLeft));
        }
        let current_node = &mut stack[len - 1];
        current_node.1 = ScanState::ScanRight;
      }
      ScanState::ScanRight => {
        if !node_ref.right_child.is_none() {
          let node_ref = node_ref.right_child.as_ref().unwrap() as &TreeNode;
          stack.push((node_ref as *const TreeNode, ScanState::ScanLeft));
        }
        let current_node = &mut stack[len - 1];
        current_node.1 = ScanState::ScanSelf;
      }
    }
  }
}
