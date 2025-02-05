pub const WRITE_TOTAL_COUNT: usize = 1000 * 10000;
pub const QUEUE_SIZE: usize = 4;
pub const WRITE_THREAD_COUNT: usize = 1;
pub const READ_THREAD_COUNT: usize = 1;
pub const WRITE_COUNT_PER_THREAD: usize = WRITE_TOTAL_COUNT / WRITE_THREAD_COUNT;
