use std::{cell::RefCell, future::Future, sync::Arc};
mod utils;

trait Callable {
  fn call(&self);
}
#[allow(dead_code)]
struct DummyCallable {
  age: i32,
}

#[derive(Debug, Clone, Copy)]
struct Dummy2Callable {
  age: i32,
  height: i32,
}

impl Callable for DummyCallable {
  fn call(&self) {
    println!("hello");
  }
}

impl Callable for Dummy2Callable {
  fn call(&self) {
    println!("hello");
  }
}

fn types_vals_size_study() {
  println!("String reference size: {}", std::mem::size_of::<&String>());
  println!("String slice size: {}", std::mem::size_of::<&str>());
  println!("Null struct size: {}", std::mem::size_of::<DummyCallable>());
  println!("Struct reference size: {}", std::mem::size_of::<&DummyCallable>());
  let caller = DummyCallable { age: 30 };
  let caller_ref: &dyn Callable = &caller;
  println!("dyn trait size: {}", std::mem::size_of::<&dyn Callable>());
  println!("dyn trait value size: {}", std::mem::size_of_val(caller_ref));
  println!("DummyCallable value size: {}", std::mem::size_of_val(&caller));
}

fn print(dummy: Dummy2Callable) {
  println!("age: {}, height: {}", dummy.age, dummy.height);
}

fn move_semantic_study() {
  let dummy = Dummy2Callable { age: 30, height: 20 };
  println!("{:?}", dummy);
  print(dummy);
  let mut dummy1 = dummy;
  dummy1.age = 40;
  print(dummy1);
  let mut x = [0; 5];
  let mut y = x;
  x[1] = 10;
  y[1] = 20;
  println!("{:?}, {:?}", x, y);
}

fn smart_ptr_study() {
  // let mut p1 = Box::new(10);
  // *p1 = 20;
  // let r2: &mut i32 = &mut p1;
  // let r1: &i32 = &p1;
  // println!("{}", r1);
  // println!("{}", r2);
  let cell = RefCell::new(10);
  {
    let mut mut_ref = cell.borrow_mut();
    *mut_ref = 20;
  }

  let v_ref = cell.borrow();
  let i = 30;
  let j = &i;
  println!("{}, {}", v_ref, j);
}

fn cocurrent_study() {
  let data = Arc::new(std::sync::RwLock::new(20));
  let data_copied = Arc::clone(&data);
  let handle = std::thread::spawn(move || {
    *data_copied.write().unwrap() = 10;
    println!(
      "Thread id: {:?}, {}",
      std::thread::current().id(),
      data_copied.read().unwrap()
    );
  });
  handle.join().unwrap();
  println!("Thread id: {:?}, {}", std::thread::current().id(), data.read().unwrap());
}

fn main() {
  types_vals_size_study();
  move_semantic_study();
  smart_ptr_study();
  cocurrent_study();
  utils::m1::m2::m3::hello();
}
