use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;
use std::thread;

pub fn multithread_study() {
  // 创建一个原子类型的计数器
  let counter = Arc::new(AtomicUsize::new(0));

  // 创建一个空的线程向量
  let mut handles = vec![];

  for _ in 0..10 {
    // 克隆计数器的Arc，以便在线程之间共享
    let counter_clone = Arc::clone(&counter);

    // 创建一个新线程
    let handle = thread::spawn(move || {
      // 更新计数器，使用`fetch_add`来增加值，确保操作的原子性
      // `Ordering::Relaxed`意味着操作可能会被重新排序，但在这个例子中没有关系
      counter_clone.fetch_add(1, Ordering::Relaxed);
    });

    // 将线程句柄推入向量中
    handles.push(handle);
  }

  // 等待所有线程完成
  for handle in handles {
    handle.join().unwrap();
  }

  // 打印最终的计数器值
  println!("Counter: {}", counter.load(Ordering::Relaxed));
}
