use std::collections::VecDeque;

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

// 构造二叉树采用层次遍历（广度优先遍历）类似的思想，借助一个队列：
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

// 层次遍历
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

// 先序遍历
pub fn dfs_pre_tree(tree_root: &Box<TreeNode>) {
  print!("{}, ", tree_root.value);
  if !tree_root.left_child.is_none() {
    dfs_pre_tree(tree_root.left_child.as_ref().unwrap());
  }
  if !tree_root.right_child.is_none() {
    dfs_pre_tree(tree_root.right_child.as_ref().unwrap());
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
