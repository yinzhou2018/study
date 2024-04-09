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
}
