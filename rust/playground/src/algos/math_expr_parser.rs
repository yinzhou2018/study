pub fn parse_math_expr(s: &str) -> i32 {
  return parse_math_expr_impl(0, b'+', s);
}

fn parse_math_expr_impl(left_value: i32, op_left: u8, s: &str) -> i32 {
  let (right_value, op_right, idx) = parse_left_and_op(s);
  if priority_greater_or_equal_than(op_left, op_right) {
    let new_left_value = eval(left_value, op_left, right_value);
    if idx < s.len() {
      return parse_math_expr_impl(new_left_value, op_right, &s[idx..]);
    } else {
      return new_left_value;
    }
  } else {
    let acc_right_value = parse_math_expr_impl(right_value, op_right, &s[idx..]);
    return eval(left_value, op_left, acc_right_value);
  }
}

fn parse_left_and_op(s: &str) -> (i32, u8, usize) {
  let mut idx: usize = 0;
  while idx < s.len() && is_num(s.as_bytes()[idx]) {
    idx = idx + 1;
  }
  let left = s[0..idx].parse::<i32>().unwrap();
  let op = if idx < s.len() { s.as_bytes()[idx] } else { b'+' };
  (left, op, idx + 1)
}

fn is_num(c: u8) -> bool {
  return c >= b'0' && c <= b'9';
}

fn priority_greater_or_equal_than(op_left: u8, op_right: u8) -> bool {
  match (op_left, op_right) {
    (b'+' | b'-', b'*' | b'/') => false,
    _ => true,
  }
}

fn eval(left_value: i32, op: u8, right_value: i32) -> i32 {
  match op {
    b'+' => left_value + right_value,
    b'-' => left_value - right_value,
    b'*' => left_value * right_value,
    b'/' => left_value / right_value,
    _ => panic!("unexpected op: {}", op),
  }
}
