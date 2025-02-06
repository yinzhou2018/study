use futures::future::*;
use futures::task::Poll::*;
use futures::task::*;

use std::pin::*;
use std::sync::mpsc::*;
use std::sync::*;
use std::time::Duration;

/// Task executor that receives tasks off of a channel and runs them.
struct Executor {
  ready_queue: Receiver<Arc<Task>>,
}

/// `Spawner` spawns new futures onto the task channel.
#[derive(Clone)]
struct Spawner {
  task_sender: SyncSender<Arc<Task>>,
}

/// A future that can reschedule itself to be polled by an `Executor`.
struct Task {
  /// In-progress future that should be pushed to completion.
  ///
  /// The `Mutex` is not necessary for correctness, since we only have
  /// one thread executing tasks at once. However, Rust isn't smart
  /// enough to know that `future` is only mutated from one thread,
  /// so we need to use the `Mutex` to prove thread-safety. A production
  /// executor would not need this, and could use `UnsafeCell` instead.
  future: Mutex<Option<BoxFuture<'static, ()>>>,
}

fn new_executor_and_spawner() -> (Executor, Spawner) {
  // Maximum number of tasks to allow queueing in the channel at once.
  // This is just to make `sync_channel` happy, and wouldn't be present in
  // a real executor.
  const MAX_QUEUED_TASKS: usize = 10_000;
  let (task_sender, ready_queue) = sync_channel(MAX_QUEUED_TASKS);
  (Executor { ready_queue }, Spawner { task_sender })
}

struct TinyWaker {
  task: Arc<Task>,
  /// Handle to place the task itself back onto the task queue.
  task_sender: SyncSender<Arc<Task>>,
}

unsafe fn clone(waker: *const ()) -> RawWaker {
  println!("clone raw waker.");
  RawWaker::new(waker, &S_RAW_WAKER_VTABLE)
}

unsafe fn wake(untyped_waker: *const ()) {
  println!("wake raw waker.");
  wake_by_ref(untyped_waker);
}

unsafe fn wake_by_ref(untyped_waker: *const ()) {
  println!("wake raw waker by ref.");
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

pub struct TimerFuture {
  shared_state: Arc<Mutex<SharedState>>,
}

/// Shared state between the future and the waiting thread
struct SharedState {
  /// Whether or not the sleep time has elapsed
  completed: bool,

  /// The waker for the task that `TimerFuture` is running on.
  /// The thread can use this after setting `completed = true` to tell
  /// `TimerFuture`'s task to wake up, see that `completed = true`, and
  /// move forward.
  waker: Option<Waker>,
}

impl Future for TimerFuture {
  type Output = ();
  fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Self::Output> {
    // Look at the shared state to see if the timer has already completed.
    let mut shared_state = self.shared_state.lock().unwrap();
    if shared_state.completed {
      Poll::Ready(())
    } else {
      // Set waker so that the thread can wake up the current task
      // when the timer has completed, ensuring that the future is polled
      // again and sees that `completed = true`.
      //
      // It's tempting to do this once rather than repeatedly cloning
      // the waker each time. However, the `TimerFuture` can move between
      // tasks on the executor, which could cause a stale waker pointing
      // to the wrong task, preventing `TimerFuture` from waking up
      // correctly.
      //
      // N.B. it's possible to check for this using the `Waker::will_wake`
      // function, but we omit that here to keep things simple.
      shared_state.waker = Some(cx.waker().clone());
      Poll::Pending
    }
  }
}

impl TimerFuture {
  /// Create a new `TimerFuture` which will complete after the provided
  /// timeout.
  pub fn new(duration: Duration) -> Self {
    let shared_state = Arc::new(Mutex::new(SharedState {
      completed: false,
      waker: None,
    }));

    // Spawn the new thread
    let thread_shared_state = shared_state.clone();
    thread::spawn(move || {
      thread::sleep(duration);
      let mut shared_state = thread_shared_state.lock().unwrap();
      // Signal that the timer has completed and wake up the last
      // task on which the future was polled, if one exists.
      shared_state.completed = true;
      if let Some(waker) = shared_state.waker.take() {
        waker.wake()
      }
    });

    TimerFuture { shared_state }
  }
}

async fn async_fn_2() -> i32 {
  println!("async_fn_2");
  TimerFuture::new(Duration::from_secs(1)).await;
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

pub fn async_await_study() {
  block_on(async_main());
}
