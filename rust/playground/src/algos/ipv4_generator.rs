pub fn generate_ipv4(s: &str) -> Vec<String> {
  let mut ip_ary = Vec::new();
  if s.len() < 4 || s.len() > 12 {
    println!("Invalid IP address: {}, length should be between 4 and 12", s);
    ip_ary
  } else {
    for i in 1..=std::cmp::min(3, s.len() - 3) {
      let ip1 = &s[0..i];
      if ip1.parse::<i32>().unwrap() > 255 {
        break;
      }
      for j in i + 1..=std::cmp::min(i + 3, s.len() - 2) {
        let ip2 = &s[i..j];
        if ip2.parse::<i32>().unwrap() > 255 {
          break;
        }
        for k in j + 1..=std::cmp::min(j + 3, s.len() - 1) {
          let ip3 = &s[j..k];
          let ip4 = &s[k..];
          if ip3.parse::<i32>().unwrap() > 255 {
            break;
          }
          if ip4.parse::<i32>().unwrap() > 255 {
            continue;
          }
          ip_ary.push(format!("{}.{}.{}.{}", ip1, ip2, ip3, ip4));
        }
      }
    }
    ip_ary
  }
}
