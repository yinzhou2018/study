use std::{marker::PhantomData, pin::Pin};

trait Callable {
  fn call(&self) -> &dyn Callable;
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
  fn call(&self) -> &dyn Callable {
    println!("hello");
    self
  }
}

impl Callable for Dummy2Callable {
  fn call(&self) -> &dyn Callable {
    println!("hello");
    self
  }
}

fn types_vals_size_study() {
  println!("box type size: {}", std::mem::size_of::<Box<Person>>());
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
  let ary = [1, 2, 3, 4];
  let p = ary.as_ptr();
  let v = unsafe { *p };
  println!("array value size: {}, {}", std::mem::size_of_val(&ary), v);
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

pub trait Summary {
  fn summarize_author(&self) -> String;

  fn summarize(&self) -> String {
    let s = self.summarize_author();
    format!("(Read more from {}...)", s)
  }
}
pub struct Post {
  pub title: String,   // 标题
  pub author: String,  // 作者
  pub content: String, // 内容
}

impl Summary for Post {
  fn summarize_author(&self) -> String {
    format!("文章{}, 作者是{}", self.title, self.author)
  }
}

pub struct Weibo {
  pub username: String,
  pub content: String,
}

impl Summary for Weibo {
  fn summarize_author(&self) -> String {
    format!("{}发表了微博{}", self.username, self.content)
  }
}

fn trait_study() {
  let post = Post {
    title: "Rust语言简介".to_string(),
    author: "Sunface".to_string(),
    content: "Rust棒极了!".to_string(),
  };
  let weibo = Weibo {
    username: "sunface".to_string(),
    content: "好像微博没Tweet好用".to_string(),
  };

  println!("Old weibo address: {:?}", &weibo as *const Weibo);
  let weibo2 = weibo;
  println!("New weibo address: {:?}", &weibo2 as *const Weibo);

  // let Weibo { username, content } = weibo;
  // println!("{}: {}", username, content);

  // println!("{}", post.summarize());
  // println!("{}", weibo.summarize());

  let trait_obj: &dyn Summary = &post;
  println!("{}", trait_obj.summarize());
}

#[derive(Debug)]
struct Slice<'a, T> {
  start: *const T,
  end: *const T,
  phantom: PhantomData<&'a T>,
}

fn create_slice<T>(ary: &[T]) -> Slice<T> {
  Slice {
    start: ary.as_ptr(),
    end: unsafe { ary.as_ptr().add(ary.len()) },
    phantom: PhantomData,
  }
}

#[derive(Debug)]
struct InnerDrop;
impl Drop for InnerDrop {
  fn drop(&mut self) {
    println!("InnerDrop destructor...");
  }
}

#[derive(Debug)]
struct DropImplWithInnerDrop {
  inner: *const InnerDrop,
  _marker: PhantomData<InnerDrop>,
}

impl Drop for DropImplWithInnerDrop {
  fn drop(&mut self) {
    println!("DropImplWithInnerDrop destructor...");
  }
}

fn lifetime_study() {
  let mut v: Vec<&str> = Vec::new();
  let s: String = "Short-lived".into();
  v.push(&s);
  drop(s);

  let slice;
  {
    let ary = [1, 2, 3];
    slice = create_slice(&ary);
    println!("{:?}", slice);
  }

  let pointer;
  {
    let inner = InnerDrop;
    pointer = &inner as *const InnerDrop;
  }
  let drop_impl_with_inner_drop = DropImplWithInnerDrop { inner: pointer, _marker: PhantomData };
  println!("{:?}", drop_impl_with_inner_drop);
}

struct Manager<'a> {
  name: String,
  age: i32,
  item: Item<'a>,
}

impl<'a> Manager<'a> {
  pub fn new(name: String, age: i32) -> Self {
    let mut parent = Self { name, age, item: unsafe { std::mem::zeroed() } };
    parent.item = Item { manager: &parent as *const Manager, marker: PhantomData };
    parent
  }

  pub fn get_item(&self) -> &Item {
    &self.item
  }
}

#[derive(Clone, Copy)]
struct Item<'a> {
  manager: *const Manager<'a>,
  marker: PhantomData<&'a Manager<'a>>,
}

struct Wrapper<'a> {
  s: &'a str,
}

impl<'a> Wrapper<'a> {
  pub fn new(s: &'a str) -> Self {
    Self { s }
  }
}

pub fn miscs_study() {
  lifetime_study();
  types_vals_size_study();
  move_semantic_study();
  unsafe_study();
  trait_study();
}
