use chrono::{Datelike, NaiveDate};

fn calc_days_and_first_wd_for_year_and_month(year: i32, month: u32) -> (u32, u32) {
  let start_date = NaiveDate::from_ymd_opt(year, month, 1).unwrap();
  let end_date = if month == 12 {
    NaiveDate::from_ymd_opt(year + 1, 1, 1).unwrap()
  } else {
    NaiveDate::from_ymd_opt(year, month + 1, 1).unwrap()
  };
  let days_in_month = (end_date - start_date).num_days() as u32;
  let first_wd = start_date.weekday().num_days_from_sunday();
  (days_in_month, first_wd)
}

const MONTHS_PER_YEAR: usize = 12;
const ROWS_PER_MONTH: usize = 8;
const COLS_PER_ROW: usize = 20;
const MONTH_GROUP_COUNT: usize = 3;

fn fill_calendar_for_year(year: i32) -> Vec<Vec<Vec<u8>>> {
  let mut calendar = vec![vec![vec![b' '; COLS_PER_ROW]; ROWS_PER_MONTH]; MONTHS_PER_YEAR];

  let month_names = [
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
  ];
  let weekday_short_names = "Su Mo Tu We Th Fr Sa";

  for (step, &name) in month_names.iter().enumerate() {
    let current_month = &mut calendar[step];

    // Fill month title
    let offset = (COLS_PER_ROW - name.len()) / 2;
    for (i, c) in name.chars().enumerate() {
      current_month[0][offset + i] = c as u8;
    }

    // Fill weekday titles
    for (i, c) in weekday_short_names.chars().enumerate() {
      current_month[1][i] = c as u8;
    }

    // Fill dates
    let (days, first_weekday) = calc_days_and_first_wd_for_year_and_month(year, (step + 1) as u32);
    let wd_offset = first_weekday as usize * 3;
    let flatten_month = current_month.iter_mut().flatten().skip(2 * COLS_PER_ROW + wd_offset);
    let mut count = first_weekday + 1;
    let days_per_week = 7;
    let days_view = (1..=days)
      .into_iter()
      .map(|day| {
        let s = if count % days_per_week == 0 { "" } else { " " };
        count += 1;
        let day_str = format!("{:2}{}", day, s);
        day_str.into_bytes()
      })
      .flatten();
    days_view.zip(flatten_month).for_each(|(ch, cell)| *cell = ch);
  }
  calendar
}

pub fn print_calendar_for_year(year: i32) {
  let calendar = fill_calendar_for_year(year);
  let calendar_ref = &calendar;

  let per_margin_width = 3;
  let total_margin_width = (MONTH_GROUP_COUNT - 1) * per_margin_width;
  let line_width = calendar[0][0].len() * MONTH_GROUP_COUNT + total_margin_width;

  let year_str = year.to_string();
  let year_width = (line_width + year_str.len()) / 2;
  println!("{:>width$}\n", year_str, width = year_width);

  // 进行一系列iter变换到可直接逐行输出为止：
  // 1. (12, 8, 20)转置为(8, 12, 20)
  // 2. 将第二维按3个一组分组升到3维(8, 4, 3, 20)
  // 3. 再次转置为(4, 8, 3, 20)
  let calendar_8_12_20 = (0..ROWS_PER_MONTH).map(|j| (0..MONTHS_PER_YEAR).map(move |i| &calendar_ref[i][j]));
  let calendar_8_4_3_20 = calendar_8_12_20.map(|view| {
    (0..MONTHS_PER_YEAR / MONTH_GROUP_COUNT)
      .map(move |i| view.clone().skip(i * MONTH_GROUP_COUNT).take(MONTH_GROUP_COUNT))
  });
  let calendar_8_4_3_20_ref = &calendar_8_4_3_20;
  let calendar_4_8_3_20 = (0..MONTHS_PER_YEAR / MONTH_GROUP_COUNT).map(|j| {
    (0..ROWS_PER_MONTH).map(move |i| {
      // 逻辑上实现calendar_8_4_3_20[i][j]操作，C++增加了一层view概念实现了operator[]，可以统一处理
      let mut view_8_4_3_20 = calendar_8_4_3_20_ref.clone().skip(i);
      let mut view_4_3_20 = view_8_4_3_20.next().unwrap().skip(j);
      let view_3_20 = view_4_3_20.next().unwrap();
      return view_3_20;
    })
  });

  for calendar_8_3_20 in calendar_4_8_3_20 {
    for calendar_3_20 in calendar_8_3_20 {
      for calendar_20 in calendar_3_20 {
        for cell in calendar_20 {
          print!("{}", *cell as char);
        }
        print!("   ");
      }
      println!();
    }
  }
}
