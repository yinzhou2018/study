use std::pin::Pin;

trait Callable {
  fn call(&self);
}
#[allow(dead_code)]
struct DummyCallable {
  age: i32,
}

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
  println!("char size: {}", std::mem::size_of::<char>());
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
  print(dummy);
}

struct Person {
  age: i32,
  age_p: *mut i32,
  // unused: PhantomPinned,
}

fn create_person(age: i32) -> Person {
  let mut p = Person {
    age,
    age_p: std::ptr::null_mut(),
    // unused: PhantomPinned{}
  };
  p.age_p = &mut p.age;
  let pinned = unsafe { Pin::new_unchecked(&p) };
  println!("{}", pinned.age);
  p
}

fn unsafe_study() {
  let p = create_person(30);
  let p1 = create_person(60);
  let v = unsafe { *p.age_p };
  println!("{:?}, {:?}, {}, {}", p.age_p, p1.age_p, v, p.age);
}

pub fn miscs_study() {
  types_vals_size_study();
  move_semantic_study();
  unsafe_study();
}
