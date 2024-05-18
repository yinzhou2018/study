mod algos;
mod miscs;
mod multithread;
mod usb;
mod web_server;
mod linked_list;

fn main() {
  miscs::miscs_study();
  algos::algos_study();
  multithread::multithread_study();
  usb::usb_study();
  // web_server::main();
  let mut list = linked_list::LinkedList::new();
  list.push_front(1);
  list.push_front(2);
  list.push_front(3);
}
