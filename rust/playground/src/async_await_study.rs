use futures::future::*;
use futures::task::*;

use std::pin::*;
use std::sync::mpsc::*;
use std::sync::*;
use std::thread;
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

  /// Handle to place the task itself back onto the task queue.
  task_sender: SyncSender<Arc<Task>>,
}

impl Drop for Task {
  fn drop(&mut self) {
    println!("Drop Task.");
  }
}

impl ArcWake for Task {
  fn wake_by_ref(arc_self: &Arc<Self>) {
    arc_self.task_sender.send(arc_self.clone()).expect("too many tasks queued");
  }
}

impl Executor {
  fn run(&self) {
    while let Ok(task) = self.ready_queue.recv() {
      // Take the future, and if it has not yet completed (is still Some),
      // poll it in an attempt to complete it.
      let mut future_slot = task.future.lock().unwrap();
      if let Some(mut future) = future_slot.take() {
        // let waker_pointer = &task as *const Arc<Task> as *const ();
        // println!("waker_pointer: {}", waker_pointer as usize);
        // let waker = unsafe { Waker::from_raw(RawWaker::new(waker_pointer, &S_RAW_WAKER_VTABLE)) };

        // Create a `LocalWaker` from the task itself
        let waker = waker_ref(&task);

        let context = &mut Context::from_waker(&waker);

        // `BoxFuture<T>` is a type alias for
        // `Pin<Box<dyn Future<Output = T> + Send + 'static>>`.
        // We can get a `Pin<&mut dyn Future + Send + 'static>`
        // from it by calling the `Pin::as_mut` method.
        if future.as_mut().poll(context).is_pending() {
          // We're not done processing the future, so put it
          // back in its task to be run again in the future.
          *future_slot = Some(future);
        }
      }
    }
  }
}

impl Spawner {
  fn spawn(&self, future: impl Future<Output = ()> + 'static + Send) {
    let future = future.boxed();
    let task = Arc::new(Task {
      future: Mutex::new(Some(future)),
      task_sender: self.task_sender.clone(),
    });
    self.task_sender.send(task).expect("too many tasks queued");
  }
}

fn new_executor_and_spawner() -> (Executor, Spawner) {
  // Maximum number of tasks to allow queueing in the channel at once.
  // This is just to make `sync_channel` happy, and wouldn't be present in
  // a real executor.
  const MAX_QUEUED_TASKS: usize = 10_000;
  let (task_sender, ready_queue) = sync_channel(MAX_QUEUED_TASKS);
  (Executor { ready_queue }, Spawner { task_sender })
}

// unsafe fn clone(untyped_waker: *const ()) -> RawWaker {
//   let weaker_pointer = untyped_waker as *const Arc<Task>;
//   let waker_ref = weaker_pointer.as_ref().unwrap();
//   let waker_cloned = Box::new(waker_ref.clone());
//   let waker_cloned_pointer = Box::into_raw(waker_cloned) as *const Arc<Task> as *const ();
//   println!("clone raw waker: {}", waker_cloned_pointer as usize);
//   RawWaker::new(untyped_waker, &S_RAW_WAKER_VTABLE)
// }

// unsafe fn wake(untyped_waker: *const ()) {
//   println!("wake raw waker.");
//   wake_by_ref(untyped_waker);
//   // let waker_pointer = untyped_waker as *mut Arc<Task>;
//   // unsafe { let _ = Box::from_raw(waker_pointer); };
// }

// unsafe fn wake_by_ref(untyped_waker: *const ()) {
//   let waker_pointer = untyped_waker as *const Arc<Task>;
//   let waker_ref = waker_pointer.as_ref().unwrap();
//   let weaker_cloned = waker_ref.clone();
//   println!(
//     "wake raw waker by ref: {}, cloned: {}",
//     untyped_waker as usize, &weaker_cloned as *const Arc<Task> as usize
//   );
//   waker_ref
//     .task_sender
//     .send(weaker_cloned)
//     .expect("too many tasks queued");
// }

// unsafe fn drop(untyped_waker: *const ()) {
//   let pointer = untyped_waker as usize;
//   println!("Drop raw waker: {}", pointer);
// }

// static S_RAW_WAKER_VTABLE: RawWakerVTable = RawWakerVTable::new(clone, wake, wake_by_ref, drop);

pub fn block_on(ft: impl Future<Output = ()> + Send + 'static) {
  let (executor, spawner) = new_executor_and_spawner();
  spawner.spawn(ft);
  drop(spawner);
  executor.run();
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
