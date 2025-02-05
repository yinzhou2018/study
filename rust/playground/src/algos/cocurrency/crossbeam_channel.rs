use std::cell::RefCell;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;
use std::thread;

use crossbeam::channel::*;

use super::configurations::*;

struct SafeVector {
  data: Vec<RefCell<usize>>,
}
unsafe impl Sync for SafeVector {}

pub fn crossbeam_channel_test(epoch: usize) {
  let read_flags = Arc::new(SafeVector {
    data: (0..WRITE_TOTAL_COUNT).map(|_| RefCell::new(0)).collect(),
  });
  let has_read_count = Arc::new(AtomicUsize::new(0));
  let (tx, rx) = bounded::<usize>(QUEUE_SIZE);

  let mut write_threads = vec![];
  let mut read_threads = vec![];

  let start = std::time::Instant::now();

  for i in 0..WRITE_THREAD_COUNT {
    let sender = tx.clone();
    write_threads.push(thread::spawn(move || {
      let index = i * WRITE_COUNT_PER_THREAD;
      let end = index + WRITE_COUNT_PER_THREAD;
      for item in index..end {
        sender.send(item).unwrap();
      }
    }));
  }

  for _ in 0..READ_THREAD_COUNT {
    let receiver = rx.clone();
    let read_flags = Arc::clone(&read_flags);
    let has_read_count = Arc::clone(&has_read_count);
    read_threads.push(thread::spawn(move || {
      while has_read_count.load(Ordering::Relaxed) < WRITE_TOTAL_COUNT {
        if let Ok(val) = receiver.try_recv() {
          read_flags.data[val].replace(1);
          has_read_count.fetch_add(1, Ordering::Relaxed);
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
    "{}. [CrossBeamChannel][{} result: {}, total written count: {}, write threads: {}, read threads: {}] spends: {}ms",
    epoch,
    if result == WRITE_TOTAL_COUNT { "OK" } else { "FAILED" },
    result,
    WRITE_TOTAL_COUNT,
    WRITE_THREAD_COUNT,
    READ_THREAD_COUNT,
    spend
  );
}
