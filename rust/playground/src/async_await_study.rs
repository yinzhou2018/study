use futures::task::Poll::*;
use futures::task::*;
use futures::*;

use std::pin::*;

struct TinyWaker {}

// struct TinyTask {
//   waker: TinyWaker,
//   root_fut: Pin<Box<dyn Future<Output = ()>>>,
// }

// struct Scheduler {
//   tasks: Vec<TinyTask>,
//   sender: Sender<usize>,
//   receiver: Receiver<usize>,
// }

unsafe fn clone(waker: *const ()) -> RawWaker {
  RawWaker::new(waker, &S_RAW_WAKER_VTABLE)
}

unsafe fn wake(untyped_waker: *const ()) {
  wake_by_ref(untyped_waker);
}

unsafe fn wake_by_ref(untyped_waker: *const ()) {
  // let waker_pointer = untyped_waker as *const TinyWaker;
  // let waker_ref = waker_pointer.as_ref().unwrap();
  // waker_ref.sender.send(waker_ref.id);
}

unsafe fn drop(_: *const ()) {
  println!("Drop raw waker.");
}

static S_RAW_WAKER_VTABLE: RawWakerVTable = RawWakerVTable::new(clone, wake, wake_by_ref, drop);

pub fn block_on(ft: impl Future<Output = ()>) {
  let raw_waker = TinyWaker {};
  let waker = unsafe {
    Waker::from_raw(RawWaker::new(
      &raw_waker as *const TinyWaker as *const (),
      &S_RAW_WAKER_VTABLE,
    ))
  };

  let mut cx = Context::from_waker(&waker);
  let mut ft = Box::pin(ft);
  loop {
    match ft.as_mut().poll(&mut cx) {
      Ready(x) => return x,
      _ => {}
    }
  }
}

async fn async_fn_2() -> i32 {
  println!("async_fn_2");
  20
}

async fn async_fn_1() -> i32 {
  println!("async_fn_1");
  let x = async_fn_2().await;
  x
}

async fn async_main() {
  let ft = async_fn_1().await;
  println!("async_main: {}", ft);
}

trait Printable {
  fn print(self: Pin<&Self>);
  fn print_2(self: Box<Self>);
  fn print_3(&self);
}

struct Printer {
  id: i32,
}

impl Printable for Printer {
  fn print(self: Pin<&Self>) {
    println!("{}", self.id);
  }

  fn print_2(self: Box<Self>) {
    println!("{}", self.id);
  }

  fn print_3(&self) {
    println!("{}", self.id);
  }
}

pub fn async_await_study() {
  block_on(async_main());
  let p = Printer { id: 10 };
  let p_pined = Box::pin(p);
  p_pined.as_ref().print();
  let p_boxed: Box<dyn Printable> = Box::new(Printer { id: 20 });
  p_boxed.print_2();
  // p_boxed.print_3();
}
