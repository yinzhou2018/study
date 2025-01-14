#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string_view>
#include <vector>

namespace views = std::views;
namespace ranges = std::ranges;
namespace chrono = std::chrono;

template <ranges::range R>
void print_range(R&& r, int depth = 0) {
  ranges::for_each(views::iota(0, depth), [](auto&& i) { std::cout << " "; });
  std::cout << "[";

  if constexpr (ranges::range<ranges::range_value_t<R>>) {
    ranges::for_each(r, [&](auto&& i) {
      std::cout << "\n";
      print_range(i, depth + 1);
    });
    std::cout << "\n";
    ranges::for_each(views::iota(0, depth), [](auto&& i) { std::cout << " "; });
  } else {
    auto is_first = true;
    ranges::for_each(r, [&](auto&& i) {
      if (!is_first) {
        std::cout << ", ";
      }
      is_first = false;
      std::cout << i;
    });
  }

  std::cout << "]" << std::endl;
}

constexpr auto operator""_y(unsigned long long year) {
  return chrono::year(static_cast<int>(year));
}
constexpr auto operator""_m(unsigned long long month) {
  return chrono::month(static_cast<int>(month));
}
constexpr auto operator""_d(unsigned long long day) {
  return chrono::day(static_cast<int>(day));
}

constexpr std::pair<int, int> calc_days_and_first_wd_for_year_and_month(int year, int month) {
  auto start_date = chrono::year(year) / chrono::month(month) / 1_d;
  auto end_date = chrono::year(month == 12 ? year + 1 : year) / chrono::month(month == 12 ? 1 : month + 1) / 1_d;
  chrono::sys_days start_sys_days{start_date};
  chrono::sys_days end_sys_days{end_date};
  auto first_wd = chrono::weekday{start_sys_days};
  return {(end_sys_days - start_sys_days).count(), first_wd.c_encoding()};
}

constexpr auto operator""_sv(const char* str, size_t len) {
  return std::string_view(str, len);
}

auto calc_calendar_for_year(int year) {
  constexpr auto months_per_year = 12;
  constexpr auto rows_per_month = 8;
  constexpr auto cols_per_row = 20;
  auto calendar = std::vector(months_per_year, std::vector(rows_per_month, std::vector<char>(cols_per_row, ' ')));

  auto month_names = {"January"_sv, "February"_sv, "March"_sv,     "April"_sv,   "May"_sv,      "June"_sv,
                      "July"_sv,    "August"_sv,   "September"_sv, "October"_sv, "November"_sv, "December"_sv};
  constexpr auto weekday_short_names = "Su Mo Tu We Th Fr Sa"_sv;
  static_assert(ranges::size(weekday_short_names) == cols_per_row);

  auto step = 0;
  ranges::for_each(month_names, [&](auto&& name) {
    auto& current_month = calendar[step++];

    // 填充月份标题
    auto month_title_iter = current_month[0].begin();
    auto offset = (cols_per_row - ranges::size(name)) / 2;
    ranges::advance(month_title_iter, offset);
    ranges::copy(name, month_title_iter);

    // 填充月份星期标题
    auto weekday_title_iter = current_month[1].begin();
    ranges::copy(weekday_short_names, weekday_title_iter);

    // 填充月份各具体日期
    auto [days, first_wd] = calc_days_and_first_wd_for_year_and_month(year, step);
    auto wd_offset = first_wd * 3;
    auto flatten_month_view = current_month | views::join | views::drop(2 * cols_per_row + wd_offset);
    auto count = first_wd + 1;
    constexpr auto days_per_weekday = 7;
    auto days_view = views::iota(1, days + 1) | views::transform([&](auto day) {
                       std::ostringstream os;
                       os << std::setw(2) << std::setfill(' ') << day;
                       if (count++ % days_per_weekday != 0) {
                         os << " ";
                       }
                       return os.str();
                     }) |
                     views::join;
    ranges::copy(days_view, flatten_month_view.begin());
  });

  return calendar;
}

void ranges_study() {
  auto calendar = calc_calendar_for_year(2025);

  // 打印1月份
  auto& month_one = calendar[1];
  ranges::for_each(month_one, [](auto&& row) {
    ranges::for_each(row, [](auto&& cell) { std::cout << cell; });
    std::cout << std::endl;
  });
}