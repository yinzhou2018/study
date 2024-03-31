pub fn print_connected_usb_devices() {
  let devices = rusb::devices().unwrap();
  println!("connected usb devices count: {}", devices.len());

  for device in devices.iter() {
    let device_desc = device.device_descriptor().unwrap();

    println!(
      "Bus {:03} Device {:03} ID {:04x}:{:04x}",
      device.bus_number(),
      device.address(),
      device_desc.vendor_id(),
      device_desc.product_id()
    );
  }
}

pub fn usb_study() {
  print_connected_usb_devices();
}
