use std::cell::RefCell;
use std::sync::atomic::{AtomicIsize, AtomicUsize, Ordering};
use std::sync::Arc;
use std::thread;

pub enum QueueResult {
  Ok = 0,
  Full = 1,
  Empty = 2,
}

// sequence用来判断当前位置是否为空，取值说明：
// 1. 当前位置非空时：取值为之前插入值时enqueue_pos的值加1
// 2. 当前位置为空时：取值为下一次要在这里插入值时的enqueue_pos值
#[repr(align(64))]
struct Element<T> {
  sequence: AtomicUsize,
  data: RefCell<Option<T>>,
}

unsafe impl<T> Sync for Element<T> {}

// enqueue_pos与dequeue_pos_会持续增长，通过与capacity取模运算来获取队列操作位置
// enqueue_pos == dequeue_pos => 空队列
// enqueue_pos - dequeue_pos == capacity => 队列满
pub struct LockFreeQueue<T> {
  capacity: usize,
  mask: usize,
  elements: Vec<Element<T>>,
  count: AtomicIsize,
  enqueue_pos: AtomicUsize,
  dequeue_pos: AtomicUsize,
}

impl<T> LockFreeQueue<T> {
  pub fn new(size: usize) -> Self {
    let mut size = size;
    if size % 2 != 0 {
      let mut tmp = 1;
      while tmp <= size {
        tmp <<= 1;
      }
      size = tmp;
    }

    let elements = (0..size)
      .map(|i| Element {
        sequence: AtomicUsize::new(i),
        data: RefCell::new(None),
      })
      .collect();

    println!("LockFreeQueue capacity = {}", size);

    Self {
      capacity: size,
      mask: size - 1,
      elements,
      count: AtomicIsize::new(0),
      enqueue_pos: AtomicUsize::new(0),
      dequeue_pos: AtomicUsize::new(0),
    }
  }

  pub fn enqueue(&self, val: T) -> QueueResult {
    let mut pos = self.enqueue_pos.load(Ordering::Relaxed);

    loop {
      let elem = &self.elements[pos & self.mask];
      let seq = elem.sequence.load(Ordering::Acquire);
      let dif = seq as isize - pos as isize;

      // dif == 0: 当前位置为空
      // dif < 0: 表示位置有数据，seq是上一轮的下一个位置，而pos是新一轮的位置，会更大
      // dif > 0: 表示有个并行入队先行成功
      if dif == 0 {
        match self
          .enqueue_pos
          .compare_exchange_weak(pos, pos + 1, Ordering::Relaxed, Ordering::Relaxed)
        {
          Ok(_) => break,
          Err(new_pos) => pos = new_pos,
        }
      } else if dif < 0 {
        if pos == self.capacity + self.dequeue_pos.load(Ordering::Relaxed) {
          return QueueResult::Full;
        }
        pos = self.enqueue_pos.load(Ordering::Relaxed);
      } else {
        pos = self.enqueue_pos.load(Ordering::Relaxed);
      }
    }

    let ref elem = self.elements[pos & self.mask];
    elem.data.replace(Some(val));
    elem.sequence.store(pos + 1, Ordering::Release);
    self.count.fetch_add(1, Ordering::Release);

    QueueResult::Ok
  }

  pub fn dequeue(&self, val: &mut Option<T>) -> QueueResult {
    let mut pos = self.dequeue_pos.load(Ordering::Relaxed);

    loop {
      let elem = &self.elements[pos & self.mask];
      let seq = elem.sequence.load(Ordering::Acquire);
      let dif = seq as isize - (pos + 1) as isize;

      // dif == 0: 当前位置有数据
      // dif < 0: 表示位置没有数据，此时seq==pos
      // dif > 0: 表示有一个并发出队先行成功
      if dif == 0 {
        match self
          .dequeue_pos
          .compare_exchange_weak(pos, pos + 1, Ordering::Relaxed, Ordering::Relaxed)
        {
          Ok(_) => break,
          Err(new_pos) => pos = new_pos,
        }
      } else if dif < 0 {
        if pos == self.enqueue_pos.load(Ordering::Relaxed) {
          return QueueResult::Empty;
        }
        pos = self.dequeue_pos.load(Ordering::Relaxed);
      } else {
        pos = self.dequeue_pos.load(Ordering::Relaxed);
      }
    }

    let ref elem = self.elements[pos & self.mask];
    *val = elem.data.take();
    elem.sequence.store(pos + self.mask + 1, Ordering::Release);
    self.count.fetch_sub(1, Ordering::Release);

    QueueResult::Ok
  }

  #[allow(dead_code)]
  pub fn size(&self) -> isize {
    self.count.load(Ordering::Relaxed)
  }

  #[allow(dead_code)]
  pub fn capacity(&self) -> usize {
    self.capacity
  }
}

const WRITE_TOTAL_COUNT: usize = 1000 * 10000;
const QUEUE_SIZE: usize = 4;
const WRITE_THREAD_COUNT: usize = 4;
const READ_THREAD_COUNT: usize = 4;
const WRITE_COUNT_PER_THREAD: usize = WRITE_TOTAL_COUNT / WRITE_THREAD_COUNT;

struct SafeVector {
  data: Vec<RefCell<usize>>,
}
unsafe impl Sync for SafeVector {}

pub fn lockfree_queue_test(epoch: usize) {
  let read_flags = Arc::new(SafeVector {
    data: (0..WRITE_TOTAL_COUNT).map(|_| RefCell::new(0)).collect(),
  });
  let has_read_count = Arc::new(AtomicUsize::new(0));
  let lf_queue = Arc::new(LockFreeQueue::<usize>::new(QUEUE_SIZE));

  let mut write_threads = vec![];
  let mut read_threads = vec![];

  let start = std::time::Instant::now();

  for i in 0..WRITE_THREAD_COUNT {
    let lf_queue = Arc::clone(&lf_queue);
    write_threads.push(thread::spawn(move || {
      let mut index = i * WRITE_COUNT_PER_THREAD;
      let end = index + WRITE_COUNT_PER_THREAD;
      while index < end {
        if let QueueResult::Ok = lf_queue.enqueue(index) {
          index += 1;
        }
      }
    }));
  }

  for _ in 0..READ_THREAD_COUNT {
    let lf_queue = Arc::clone(&lf_queue);
    let read_flags = Arc::clone(&read_flags);
    let has_read_count = Arc::clone(&has_read_count);
    read_threads.push(thread::spawn(move || {
      while has_read_count.load(Ordering::Relaxed) < WRITE_TOTAL_COUNT {
        let mut val = None;
        if let QueueResult::Ok = lf_queue.dequeue(&mut val) {
          if let Some(val) = val {
            read_flags.data[val].replace(1);
            has_read_count.fetch_add(1, Ordering::Relaxed);
          }
        }
      }
    }));
  }

  for t in write_threads {
    t.join().unwrap();
  }
  for t in read_threads {
    t.join().unwrap();
  }

  let end = std::time::Instant::now();
  let spend = end.duration_since(start).as_millis();
  let result = read_flags.data.iter().map(|x| *x.borrow()).fold(0, |acc, x| acc + x);
  println!(
      "{}. [{} result: {}, total written count: {}, write threads: {}, read threads: {}] lockfree_queue_test spends: {}ms",
      epoch,
      if result == WRITE_TOTAL_COUNT { "OK" } else { "FAILED" },
      result,
      WRITE_TOTAL_COUNT,
      WRITE_THREAD_COUNT,
      READ_THREAD_COUNT,
      spend
    );
}
