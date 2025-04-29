use std::cell::RefCell;
use std::sync::{atomic::*, *};
use std::time::Instant;

struct TaskInfo {
  task: Box<dyn Fn() + Send + 'static>,
  enter_time: Instant,
  interval_ms: i32,
}

pub struct Timer {
  tasks: Arc<Mutex<Vec<TaskInfo>>>,
  running: Arc<AtomicBool>,
  cvar: Arc<Condvar>,
  handle: RefCell<Option<std::thread::JoinHandle<()>>>,
}

impl Timer {
  pub fn new() -> Self {
    let tasks = Arc::new(Mutex::new(Vec::new()));
    let running = Arc::new(AtomicBool::new(false));
    let cvar = Arc::new(Condvar::new());
    Timer {
      tasks,
      running,
      cvar,
      handle: RefCell::new(None),
    }
  }

  pub fn add_task<F>(&self, interval_ms: i32, task: F)
  where
    F: Fn() + Send + 'static,
  {
    let task_info = TaskInfo {
      task: Box::new(task),
      enter_time: Instant::now(),
      interval_ms,
    };
    let mut tasks = self.tasks.lock().unwrap();
    tasks.push(task_info);
    self.cvar.notify_one();
  }

  pub fn start(&self) {
    if self.running.load(Ordering::Relaxed) {
      return;
    }

    self.running.store(true, Ordering::SeqCst);
    let tasks = self.tasks.clone();
    let running = self.running.clone();
    let cvar = self.cvar.clone();

    let handle = std::thread::spawn(move || {
      {
        let tasks = tasks.lock().unwrap();
        if tasks.is_empty() {
          let __ = cvar.wait(tasks).unwrap();
        }
      }

      let mut local_tasks: Vec<TaskInfo> = Vec::new();
      while running.load(Ordering::Relaxed) {
        {
          let mut tasks = tasks.lock().unwrap();
          local_tasks.extend(tasks.drain(..));
        }

        let now = Instant::now();
        for info in local_tasks.iter_mut() {
          if now.duration_since(info.enter_time).as_millis() as i32 >= info.interval_ms {
            info.enter_time = now;
            (info.task)();
          }
        }
        std::thread::sleep(std::time::Duration::from_millis(1));
      }
    }); // spawn
    self.handle.replace(Some(handle));
  }

  pub fn stop(&self) {
    if !self.running.load(Ordering::Relaxed) {
      return;
    }

    self.running.store(false, Ordering::SeqCst);
    self.cvar.notify_one();
    if let Some(handle) = self.handle.borrow_mut().take() {
      handle.join().unwrap();
    }
  }
} // impl Timer

impl Drop for Timer {
  fn drop(&mut self) {
    self.stop();
  }
}

pub fn timer_study() {
  println!("Timer study");
  let timer = Timer::new();
  timer.start();
  timer.add_task(100, || {
    println!("Hello, world! task 1");
  });
  timer.add_task(200, || {
    println!("Hello, world! task 2");
  });
  timer.add_task(300, || {
    println!("Hello, world! task 3");
  });
  std::thread::sleep(std::time::Duration::from_secs(10));
  timer.stop();
}