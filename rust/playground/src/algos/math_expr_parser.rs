pub fn parse_math_expr(expr: &str) -> i32 {
  let postfix_expr = convert_to_postfix_expr(expr);
  let plus = '+' as i32;
  let mul = '*' as i32;
  let div = '/' as i32;
  let minus = '-' as i32;
  println!("midfix expr: {}", expr);
  print!("postfix expr: ");
  postfix_expr.iter().for_each(|e| {
    if *e == plus || *e == minus || *e == mul || *e == div {
      print!("{}", char::from_u32(*e as u32).unwrap());
    } else {
      print!("{}", e);
    }
  });
  print!("\n");

  let mut stack = Vec::new();
  for e in &postfix_expr {
    if *e == plus || *e == minus || *e == mul || *e == div {
      let right = stack.pop().unwrap();
      let left = stack.pop().unwrap();
      let result = eval(left, *e as u8, right);
      stack.push(result);
    } else {
      stack.push(*e);
    }
  }

  return stack.pop().unwrap();
}

fn convert_to_postfix_expr(expr: &str) -> Vec<i32> {
  let mut result = Vec::new();
  let mut op_stack = Vec::new();
  let mut idx = 0;

  while idx < expr.len() {
    let mut end_idx = idx;
    while end_idx < expr.len() && is_num(expr.as_bytes()[end_idx]) {
      end_idx = end_idx + 1;
    }

    if end_idx > idx {
      let num = expr[idx..end_idx].parse::<i32>().unwrap();
      result.push(num);
      idx = end_idx;
    } else {
      let op = expr.as_bytes()[idx];
      match op {
        b'(' => op_stack.push(op),
        b')' => {
          while op_stack[op_stack.len() - 1] != b'(' {
            result.push(op_stack.pop().unwrap() as i32);
          }
          op_stack.pop();
        }
        b'*' | b'/' | b'+' | b'-' => {
          if op_stack.len() > 0 {
            match op_stack[op_stack.len() - 1] {
              b'*' | b'/' => result.push(op_stack.pop().unwrap() as i32),
              _ => (),
            }
          }
          op_stack.push(op);
        }
        _ => panic!("unexpected op: {}", op),
      }
      idx = idx + 1;
    }
  }

  while op_stack.len() > 0 {
    result.push(op_stack.pop().unwrap() as i32);
  }

  result
}

pub fn parse_math_expr_without_parentheses(expr: &str) -> i32 {
  return parse_math_expr_impl(0, b'+', expr);
}

fn parse_math_expr_impl(left_value: i32, left_op: u8, expr: &str) -> i32 {
  let (right_value, right_op, idx) = parse_left_and_op(expr);
  if priority_greater_or_equal_than(left_op, right_op) {
    let new_left_value = eval(left_value, left_op, right_value);
    if idx < expr.len() {
      return parse_math_expr_impl(new_left_value, right_op, &expr[idx..]);
    } else {
      return new_left_value;
    }
  } else {
    let acc_right_value = parse_math_expr_impl(right_value, right_op, &expr[idx..]);
    return eval(left_value, left_op, acc_right_value);
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

fn priority_greater_or_equal_than(left_op: u8, right_op: u8) -> bool {
  match (left_op, right_op) {
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
