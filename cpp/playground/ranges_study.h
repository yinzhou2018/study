#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <iomanip>
#include <ranges>
#include <string_view>
#include <vector>

namespace views = std::views;
namespace ranges = std::ranges;
namespace chrono = std::chrono;

template <ranges::input_range V>
  requires ranges::view<V>
class group_view : public std::ranges::view_interface<group_view<V>> {
 public:
  class iterator {
   private:
    ranges::iterator_t<V> current_;
    ranges::range_difference_t<V> group_size_;

   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = ranges::subrange<ranges::iterator_t<V>, ranges::iterator_t<V>>;
    using difference_type = ranges::range_difference_t<V>;

    iterator() = default;
    iterator(ranges::iterator_t<V> current, ranges::range_difference_t<V> group_size)
        : current_(current), group_size_(group_size) {}

    iterator& operator++() {
      ranges::advance(current_, group_size_);
      return *this;
    }

    iterator operator++(int) {
      iterator temp = *this;
      ++(*this);
      return temp;
    }

    decltype(auto) operator*() const {
      auto end = current_;
      ranges::advance(end, group_size_);
      return ranges::subrange(current_, end);
    }

    bool operator==(const iterator& other) const {
      return current_ == other.current_ && group_size_ == other.group_size_;
    }
    bool operator!=(const iterator& other) const { return !(*this == other); }
  };  // iterator

  group_view() = default;
  group_view(V base, ranges::range_difference_t<V> group_size) : base_(std::move(base)), group_size_(group_size) {}

  auto begin() { return iterator(ranges::begin(base_), group_size_); }
  auto end() { return iterator(ranges::end(base_), group_size_); }
  decltype(auto) operator[](ranges::range_difference_t<V> group_size) {
    auto iter = begin();
    ranges::advance(iter, group_size);
    return *iter;
  }

 private:
  V base_;
  ranges::range_difference_t<V> group_size_;
};

template <typename Range>
group_view(Range&&, ranges::range_difference_t<Range>) -> group_view<views::all_t<Range>>;

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

constexpr auto months_per_year = 12;
constexpr auto rows_per_month = 8;
constexpr auto cols_per_row = 20;

auto fill_calendar_for_year(int year) {
  auto calendar = std::vector(months_per_year, std::vector(rows_per_month, std::vector(cols_per_row, ' ')));

  auto month_names = {"January"_sv, "February"_sv, "March"_sv,     "April"_sv,   "May"_sv,      "June"_sv,
                      "July"_sv,    "August"_sv,   "September"_sv, "October"_sv, "November"_sv, "December"_sv};
  constexpr auto weekday_short_names = "Su Mo Tu We Th Fr Sa"_sv;
  static_assert(ranges::size(weekday_short_names) == cols_per_row);

  // 遍历逐月填充标题及具体日期
  auto step = 0;
  ranges::for_each(month_names, [&](auto&& name) {
    auto&& current_month = calendar[step++];

    // 填充月份标题
    auto month_title_iter = current_month[0].begin();
    auto offset = (cols_per_row - ranges::size(name)) / 2;
    ranges::advance(month_title_iter, offset);
    ranges::copy(name, month_title_iter);

    // 填充星期标题
    auto weekday_title_iter = current_month[1].begin();
    ranges::copy(weekday_short_names, weekday_title_iter);

    // 填充具体日期
    auto [days, first_weekday] = calc_days_and_first_wd_for_year_and_month(year, step);
    auto wd_offset = first_weekday * 3;
    auto flatten_month_view = current_month | views::join | views::drop(2 * cols_per_row + wd_offset);
    auto count = first_weekday + 1;
    constexpr auto days_per_week = 7;
    auto days_view = views::iota(1, days + 1) | views::transform([&](auto day) {
                       std::ostringstream os;
                       os << std::setw(2) << std::setfill(' ') << day;
                       if (count++ % days_per_week != 0) {
                         os << " ";
                       }
                       return os.str();
                     }) |
                     views::join;
    ranges::copy(days_view, flatten_month_view.begin());
  });

  return calendar;
}

void print_calender_for_year(int year) {
  auto calendar = fill_calendar_for_year(year);

  constexpr auto month_group_count = 3;
  constexpr auto per_margin_width = 3;
  constexpr auto total_margin_width = (month_group_count - 1) * per_margin_width;
  auto line_width = calendar[0][0].size() * month_group_count + total_margin_width;

  auto year_str = std::to_string(year);
  auto year_width = (line_width + year_str.size()) / 2;
  std::cout << std::setw(year_width) << std::setfill(' ') << year_str << "\n\n";

  // 进行一系列view变换到可直接逐行输出为止：
  // 1. (12, 8, 20)转置为(8, 12, 20)
  // 2. 将第二维按3个一组分组升到3维(8, 4, 3, 20)
  // 3. 再次转置为(4, 8, 3, 20)
  auto calendar_8_12_20 = views::iota(0, rows_per_month) | views::transform([&](auto j) {
                            auto view_12_20 = views::iota(0, months_per_year) |
                                              views::transform([&, _j = j](auto&& i) { return calendar[i][_j]; });
                            return view_12_20;
                          });
  auto calender_8_4_3_20 = calendar_8_12_20 | views::transform([&](auto&& view) { return group_view(view, 3); });
  auto calender_4_8_3_20 = views::iota(0, 4) | views::transform([&](auto j) {
                             auto view_8_3_20 = views::iota(0, 8) | views::transform([&, _j = j](auto i) {
                                                  static auto view_4_3_20 = calender_8_4_3_20[0];
                                                  view_4_3_20 = calender_8_4_3_20[i];
                                                  return view_4_3_20[_j];
                                                });
                             return view_8_3_20;
                           });

  ranges::for_each(calender_4_8_3_20, [](auto&& calender_8_3_20) {
    ranges::for_each(calender_8_3_20, [](auto&& calender_3_20) {
      ranges::for_each(calender_3_20, [](auto&& calender_20) {
        ranges::for_each(calender_20, [](auto cell) { std::cout << cell; });
        std::cout << "   ";
      });
      std::cout << std::endl;
    });
  });
}

void ranges_study() {
  print_calender_for_year(2024);
}