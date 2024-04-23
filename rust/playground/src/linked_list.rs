use std::ptr::{self};

struct Node<T> {
  value: T,
  next: Option<Box<Node<T>>>,
}

// impl<T> Drop for Node<T> {
//   fn drop(&mut self) {
//     println!("Node destructor...");
//   }
// }

pub struct LinkedList<T> {
  head: Option<Box<Node<T>>>,
  tail: *mut Node<T>,
  size: i32,
}

impl<T> LinkedList<T> {
  pub fn new() -> Self {
    LinkedList { head: None, tail: ptr::null_mut(), size: 0 }
  }

  pub fn front(&self) -> Option<&T> {
    let node = self.head.as_ref()?;
    Some(&node.value)
  }

  pub fn back(&self) -> Option<&T> {
    let node = unsafe { self.tail.as_ref()? };
    Some(&node.value)
  }

  pub fn push_front(&mut self, value: T) {
    self.size += 1;
    let mut node = Box::new(Node { value, next: self.head.take() });
    self.head = Some(node);
    if self.tail.is_null() {
      self.tail = self.head.as_mut().unwrap().as_mut();
    }
  }

  pub fn push_back(&mut self, value: T) {
    if self.tail.is_null() {
      self.push_front(value);
    } else {
      self.size += 1;
      let node = Box::new(Node { value, next: None });
      unsafe { self.tail.as_mut().unwrap().next = Some(node) };
      self.tail = unsafe { self.tail.as_mut().unwrap().next.as_mut().unwrap().as_mut() };
    }
  }

  pub fn pop_front(&mut self) -> Option<T> {
    if self.head.is_none() {
      None
    } else {
      self.size -= 1;
      let node = self.head.take()?;
      self.head = node.next;
      if self.head.is_none() {
        self.tail = ptr::null_mut();
      }
      Some(node.value)
    }
  }

  pub fn pop_back(&mut self) -> Option<T> {
    if self.head.is_none() {
      None
    } else {
      self.size -= 1;
      if self.size == 0 {
        let node = self.head.take().unwrap();
        self.tail = ptr::null_mut();
        Some(node.value)
      } else {
        let mut node = self.head.as_mut().unwrap();
        while node.next.as_ref().unwrap().next.is_some() {
          node = node.next.as_mut().unwrap();
        }
        let return_node = node.next.take().unwrap();
        self.tail = node.as_mut();
        Some(return_node.value)
      }
    }
  }
}

#[cfg(test)]
mod tests {
  use super::*;
  
  #[test]
  fn test_linked_list() {
    let mut list = LinkedList::new();
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);
    assert_eq!(list.front(), Some(&3));
    assert_eq!(list.back(), Some(&1));
    assert_eq!(list.pop_front(), Some(3));
    assert_eq!(list.pop_front(), Some(2));
    assert_eq!(list.pop_front(), Some(1));
    assert_eq!(list.pop_front(), None);
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    assert_eq!(list.front(), Some(&1));
    assert_eq!(list.back(), Some(&3));
    assert_eq!(list.pop_back(), Some(3));
    assert_eq!(list.pop_back(), Some(2));
    assert_eq!(list.pop_back(), Some(1));
    assert_eq!(list.pop_back(), None);
  }
}
